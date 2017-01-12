// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Atlases/AtlasCompiler.h"
#include "Builder/Builder/AssetBuilder.h"
#include "Engine/Platform/Platform.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Textures/TextureFactory.h"

#include "Engine/Resources//Compiled/Atlases/CompiledAtlas.h"

#include "libsquish/squish.h"

#include "Engine/IO/PatchedBinaryStream.h"

#include <algorithm>

void AtlasCompiler::Expand_Multiple_Path(AssetBuilder* builder, const char* path, std::vector<std::string>& result)
{
	std::string file_path = path;

	int hash_mark = file_path.find('#');
	DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'multiple' does not contain replacement hash mark.");

	std::string split_left  = file_path.substr(0, hash_mark);
	std::string split_right = file_path.substr(hash_mark + 1);

	int counter = 0;
	while (true)
	{
		std::string dep_path = split_left + StringHelper::To_String(counter++) + split_right;
		std::string resolved_path = builder->Resolve_Asset_Path(dep_path);

		if (Platform::Get()->Is_File(resolved_path.c_str()))
		{
			result.push_back(dep_path);
		}
		else
		{
			break;
		}
	}
}

AnimationMode::Type AtlasCompiler::Parse_Animation_Mode(const char* mode)
{
	AnimationMode::Type		 result		= (AnimationMode::Type)0;
	std::vector<std::string> segments;

	StringHelper::Split(mode, '|', segments);

	for (std::vector<std::string>::iterator iter = segments.begin(); iter != segments.end(); iter++)
	{
		std::string& seg = *iter;

		if (stricmp(seg.c_str(), "once") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::Once);
		}
		else if (stricmp(seg.c_str(), "loop") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::Loop);
		}
		else if (stricmp(seg.c_str(), "ping_pong") == 0)
		{
			result = (AnimationMode::Type)((int)result | (int)AnimationMode::PingPong);
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid animation mode '%s'", seg.c_str());
		}
	}

	return result;
}

int AtlasCompiler::Get_Version()
{
	return 23;
}

std::vector<AssetToCompile> AtlasCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("atlases"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("atlases/atlas");

	// Load atlases.
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		AssetToCompile asset;
		asset.Source_Path = file->Get<const char*>("file", node, true);
		asset.Priority = "";

		if (file->Contains("priority", node, true))
		{
			asset.Priority = file->Get<const char*>("priority", node, true);
		}

		// Calculate all dependencies.
		std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
		
		ConfigFile asset_file;
		if (!asset_file.Load(resolved_asset_path.c_str()))
		{
			DBG_LOG("[WARNING] Failed to check dependencies for atlas. Atlas file does not exist! '%s'.", asset.Source_Path.c_str());
			continue;
		}

		std::vector<ConfigFileNode> images = asset_file.Get<std::vector<ConfigFileNode> >("images/image");
		for (std::vector<ConfigFileNode>::iterator iter = images.begin(); iter != images.end(); iter++)
		{
			ConfigFileNode node = *iter;		

			const char* type = asset_file.Get<const char*>("type", node, true);
			const char* path = asset_file.Get<const char*>("file", node, true);

			if (stricmp(type, "single") == 0)
			{
				asset.Dependent_Files.push_back(path);
			}		
			else if (stricmp(type, "multiple") == 0)
			{
				Expand_Multiple_Path(builder, path, asset.Dependent_Files);
			}
			else if (stricmp(type, "atlas") == 0)
			{
				asset.Dependent_Files.push_back(path);
			}
			else if (stricmp(type, "grid") == 0)
			{
				asset.Dependent_Files.push_back(path);
			}
			else
			{
				DBG_ASSERT_STR(false, "Unknown or invalid source-image type '%s'", type);
			}
		}

		result.push_back(asset);
	}

	return result;
}

struct Atlas_Build_Frame
{
	Pixelmap* pixmap;
	std::string name;
	Rect2D rect;
	Vector2 origin;
	Rect2D grid_origin;
	float area;
	int index;
	std::vector<AtlasMultiLayerFrame> ml_frames;
	AtlasFrame* frame;
	float depth_bias;

	Atlas_Build_Frame(const char* p_name, Pixelmap* p_map, Rect2D p_rect, Vector2 p_origin, float p_depth_bias, Rect2D p_grid_origin = Rect2D(0.0f, 0.0f, 0.0f, 0.0f))
		: name(p_name)
		, pixmap(p_map)
		, rect(p_rect)
		, origin(p_origin)
		, grid_origin(p_grid_origin)
		, depth_bias(p_depth_bias)
	{
		area = rect.Width * rect.Height;
	}

	static bool Sort_Predicate(Atlas_Build_Frame* a, Atlas_Build_Frame* b)
	{
		return Max(a->rect.Width, a->rect.Height) > Max(b->rect.Width, b->rect.Height);//a->area > b->area;
	}
};

struct Atlas_Build_Anim
{
	std::string							name;
	float								speed;
	AnimationMode::Type					mode;
	std::vector<Atlas_Build_Frame*>		frames;
};


bool AtlasCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	//DBG_LOG("Compiling atlas resource '%s'.", asset.Source_Path.c_str());

	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	// Load asset file.
	ConfigFile asset_file;
	if (!asset_file.Load(resolved_asset_path.c_str()))
	{
		return false;
	}

	// Create new atlas, and start adding rectangles.
	Atlas* atlas = new Atlas(&asset_file);
	atlas->Lock_Textures();

	std::vector<Atlas_Build_Frame*> add_frames;
	std::vector<Atlas_Build_Anim> add_anims;
	std::vector<Pixelmap*> loaded_pixmaps;

	//DBG_LOG("Getting images ...");

	// Add each of the source-images.
	std::vector<ConfigFileNode> images = asset_file.Get<std::vector<ConfigFileNode> >("images/image");
	for (std::vector<ConfigFileNode>::iterator iter = images.begin(); iter != images.end(); iter++)
	{
		ConfigFileNode node = *iter;		

		const char* type = asset_file.Get<const char*>("type", node, true);
		const char* file = asset_file.Get<const char*>("file", node, true);

		std::string resolved_file = builder->Resolve_Asset_Path(file);

		Vector2 origin = Vector2(0, 0);
		float depthbias = 0.0f;

		int start_frame_index = add_frames.size();

		if (asset_file.Contains("origin", node, true))
		{
			origin = asset_file.Get<Vector2>("origin", node, true);
		}

		if (asset_file.Contains("depthbias", node, true))
		{
			depthbias = asset_file.Get<float>("depthbias", node, true);
		}

		if (stricmp(type, "single") == 0)
		{
			const char* name = asset_file.Get<const char*>("name", node, true);

			Pixelmap* pixelmap = PixelmapFactory::Load(resolved_file.c_str());
			DBG_ASSERT_STR(pixelmap != NULL, "Could not load dependent texture '%s'.", resolved_file.c_str());
			loaded_pixmaps.push_back(pixelmap);

			add_frames.push_back(new Atlas_Build_Frame(name, pixelmap, Rect2D(0.0f, 0.0f, (float)pixelmap->Get_Width(), (float)pixelmap->Get_Height()), origin, depthbias));
		}		
		else if (stricmp(type, "multiple") == 0)
		{
			const char* name = asset_file.Get<const char*>("name", node, true);

			// Grab all the file paths.
			std::string name_str = name;
			std::vector<std::string> files;
			Expand_Multiple_Path(builder, file, files);

			// Grab hash mark.
			int hash_mark = name_str.find('#');
			DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'multiple' does not contain replacement hash mark.");
			std::string split_left  = name_str.substr(0, hash_mark);
			std::string split_right = name_str.substr(hash_mark + 1);

			// Add each file.
			int counter = 0;
			for (std::vector<std::string>::iterator subiter = files.begin(); subiter != files.end(); subiter++)
			{
				std::string sub_path = *subiter;
				std::string sub_name = split_left + StringHelper::To_String(counter++) + split_right;

				Pixelmap* pixelmap = PixelmapFactory::Load(sub_path.c_str());
				DBG_ASSERT_STR(pixelmap != NULL, "Could not load dependent texture '%s'.", sub_path.c_str());
				loaded_pixmaps.push_back(pixelmap);

				add_frames.push_back(new Atlas_Build_Frame(sub_name.c_str(), pixelmap, Rect2D(0, 0, pixelmap->Get_Width(), pixelmap->Get_Height()), origin, depthbias));
			}
		}
		else if (stricmp(type, "grid") == 0)
		{
			const char* name = asset_file.Get<const char*>("name", node, true);

			Pixelmap* pixelmap = PixelmapFactory::Load(resolved_file.c_str());
			DBG_ASSERT_STR(pixelmap != NULL, "Could not load dependent texture '%s'.", resolved_file.c_str());
			loaded_pixmaps.push_back(pixelmap);

			// Grab hash mark.
			std::string name_str = name;
			int hash_mark = name_str.find('#');
			DBG_ASSERT_STR(hash_mark != std::string::npos, "Image marked as type 'grid' does not contain replacement hash mark.");
			std::string split_left  = name_str.substr(0, hash_mark);
			std::string split_right = name_str.substr(hash_mark + 1);

			// Iterate over each sub-image.			
			int   frame_width		= asset_file.Get<int>("width", node, true);
			int   frame_height		= asset_file.Get<int>("height", node, true);
			int   frame_hspace		= asset_file.Get<int>("hspace", node, true);
			int   frame_vspace		= asset_file.Get<int>("vspace", node, true);			
			float texture_width		= (float)pixelmap->Get_Width();
			float texture_height	= (float)pixelmap->Get_Height();
			int	  cell_count_u		= (int)(texture_width  / (frame_width + frame_hspace));
			int	  cell_count_v		= (int)(texture_height / (frame_height + frame_vspace));
			int	  cell_count		= cell_count_u * cell_count_v;

			for (int i = 0; i < cell_count; i++)
			{
				int u = i % cell_count_u;
				int v = i / cell_count_u;

				int u_offset = (int)floorf((float)u * (frame_width + frame_hspace));
				int v_offset = (int)floorf((float)v * (frame_height + frame_vspace));

				std::string sub_name = split_left + StringHelper::To_String(i) + split_right;
				add_frames.push_back(new Atlas_Build_Frame(sub_name.c_str(), pixelmap, Rect2D((float)u_offset, (float)v_offset, (float)frame_width, (float)frame_height), origin, depthbias, Rect2D(u, v, cell_count_u, cell_count_v)));
			}
		}
		else if (stricmp(type, "atlas") == 0)
		{
			Pixelmap* pixelmap = PixelmapFactory::Load(resolved_file.c_str());
			DBG_ASSERT_STR(pixelmap != NULL, "Could not load dependent texture '%s'.", resolved_file.c_str());
			loaded_pixmaps.push_back(pixelmap);

			// Calculate frame positions for atlas.
			int frame_width			= asset_file.Get<int>("width", node, true);
			int frame_height		= asset_file.Get<int>("height", node, true);
			int frame_hspace		= asset_file.Get<int>("hspace", node, true);
			int frame_vspace		= asset_file.Get<int>("vspace", node, true);	
			float texture_width		= (float)pixelmap->Get_Width();
			float texture_height	= (float)pixelmap->Get_Height();
			int	  cell_count_u		= (int)(texture_width  / (frame_width + frame_hspace));
			int	  cell_count_v		= (int)(texture_height / (frame_height + frame_vspace));
			int	  cell_count		= cell_count_u * cell_count_v;

			// Iterate over each sub-image.	
			std::vector<ConfigFileNode> sub_images = asset_file.Get<std::vector<ConfigFileNode> >("subimage", node);
			for (std::vector<ConfigFileNode>::iterator subiter = sub_images.begin(); subiter != sub_images.end(); subiter++)
			{
				ConfigFileNode sub_node = *subiter;	

				const char* sub_type = asset_file.Get<const char*>("type", sub_node, true);
				const char* sub_name = asset_file.Get<const char*>("name", sub_node, true);

				if (stricmp(sub_type, "single") == 0)
				{
					int frame		= asset_file.Get<int>("frame", sub_node, true);
					int u			= frame % cell_count_u;
					int v			= frame / cell_count_u;
					int u_offset	= (int)floorf((float)u * (frame_width + frame_hspace));
					int v_offset	= (int)floorf((float)v * (frame_height + frame_vspace));

					add_frames.push_back(new Atlas_Build_Frame(sub_name, pixelmap, Rect2D(u_offset, v_offset, frame_width, frame_height), origin, depthbias));
				}
				else if (stricmp(sub_type, "strip") == 0)
				{
					int to_frame   = asset_file.Get<int>("to_frame", sub_node, true);
					int from_frame = asset_file.Get<int>("from_frame", sub_node, true);
					DBG_ASSERT_STR(to_frame >= from_frame, "Invalid strip range %i to %i.", from_frame, to_frame);

					std::string sub_name_str	= sub_name;
					int			hash_mark		= sub_name_str.find('#');
					DBG_ASSERT_STR(hash_mark != std::string::npos, "Sub-image marked as type 'strip' does not contain replacement hash mark.");

					std::string split_left  = sub_name_str.substr(0, hash_mark);
					std::string split_right = sub_name_str.substr(hash_mark + 1);

					for(int frame = from_frame; frame <= to_frame; frame++)
					{
						int u			= frame % cell_count_u;
						int v			= frame / cell_count_u;
						int u_offset	= (int)floorf((float)u * (frame_width + frame_hspace));
						int v_offset	= (int)floorf((float)v * (frame_height + frame_vspace));

						std::string sub_sub_name = split_left + StringHelper::To_String(frame) + split_right;
						add_frames.push_back(new Atlas_Build_Frame(sub_sub_name.c_str(), pixelmap, Rect2D((float)u_offset, (float)v_offset, (float)frame_width, (float)frame_height), origin, depthbias));
					}
				}
				else
				{
					DBG_ASSERT_STR(false, "Unknown or invalid sub-image type '%s'", sub_type);
				}
			}
		}
		else
		{
			DBG_ASSERT_STR(false, "Unknown or invalid source-image type '%s'", type);
		}

		// Grab the ending frame index for relative-animation calculation.
		int end_frame_index = add_frames.size();
		int frame_count = (end_frame_index - start_frame_index);

		// Load relative-animations.	
		if (asset_file.Contains("animations", node, false))
		{
			std::vector<ConfigFileNode> animations = asset_file.Get<std::vector<ConfigFileNode> >("animations/animation", node, false);
			for (std::vector<ConfigFileNode>::iterator subiter = animations.begin(); subiter != animations.end(); subiter++)
			{
				ConfigFileNode subnode = *subiter;		

				const char* name		= asset_file.Get<const char*>("name", subnode, true);
				const char* mode		= asset_file.Get<const char*>("mode", subnode, true);
				float	    speed		= asset_file.Get<float>("speed", subnode, true);
				int		    from_frame	= asset_file.Get<int>("from_frame", subnode, true);
				int		    to_frame	= asset_file.Get<int>("to_frame", subnode, true);

				DBG_ASSERT_STR(from_frame <= to_frame && from_frame >= 0 && from_frame < (end_frame_index - start_frame_index), "Invalid relative animation ('%s') from-frame index '%i'", name, from_frame);
				DBG_ASSERT_STR(to_frame >= 0 && to_frame < frame_count, "Invalid relative animation ('%s') to-frame index '%i'", name, to_frame);

				// Decode the animation mode.
				Atlas_Build_Anim a;
				for (int i = from_frame; i <= to_frame; i++)
				{
					a.frames.push_back(add_frames.at(start_frame_index + i));
				}

				// Add the animation.
				a.name = name;
				a.speed = speed;
				a.mode = Parse_Animation_Mode(mode);

				add_anims.push_back(a);
			}	
		}

		// Load multi-layer frames.
		if (asset_file.Contains("multi_layer_frames", node, false))
		{
			std::vector<ConfigFileNode> animations = asset_file.Get<std::vector<ConfigFileNode> >("multi_layer_frames/multi_layer_frame", node, false);
			for (std::vector<ConfigFileNode>::iterator subiter = animations.begin(); subiter != animations.end(); subiter++)
			{
				ConfigFileNode subnode = *subiter;		

				int	layer_index		= asset_file.Get<int>("layer_index", subnode, true);
				int	frame_offset	= asset_file.Get<int>("frame_offset", subnode, true);

				DBG_ASSERT(start_frame_index + frame_offset >= 0 && start_frame_index + frame_offset < (int)add_frames.size());

				// Add multilayer to each frame in animation.
				for (int k = 0; k < frame_count; k++)
				{
					// Add the animation.
					Atlas_Build_Frame* frame = add_frames.at(start_frame_index + k);

					AtlasMultiLayerFrame mlf;
					mlf.FrameOffset = frame_offset;
					mlf.LayerOffset = layer_index;
					mlf.Frame = NULL;

					frame->ml_frames.push_back(mlf);
				}
			}	
		}
	}

	// Sort frames by area and add to atlas. Largest to smallest produces the best packing.
	//DBG_LOG("Sorting atlas frames ...");
	std::vector<Atlas_Build_Frame*> unsorted_frames = add_frames;
	std::sort(add_frames.begin(), add_frames.end(), &Atlas_Build_Frame::Sort_Predicate);
	//DBG_LOG("Packing atlas ...");
	int index = 0;
	for (std::vector<Atlas_Build_Frame*>::iterator iter = add_frames.begin(); iter != add_frames.end(); iter++, index++)
	{
		Atlas_Build_Frame& frame = *(*iter);
		frame.index = index;
		atlas->Add_Frame(frame.name.c_str(), frame.pixmap, frame.rect, frame.origin, frame.depth_bias, frame.grid_origin);

		frame.frame = atlas->Get_Frame_By_Index(index);
		//DBG_LOG("Packed Index %i",index);
	}

	// Add multi layer frames.
	index = 0;
	for (std::vector<Atlas_Build_Frame*>::iterator iter = unsorted_frames.begin(); iter != unsorted_frames.end(); iter++, index++)
	{
		Atlas_Build_Frame* frame = (*iter);

		for (std::vector<AtlasMultiLayerFrame>::iterator iter2 = frame->ml_frames.begin(); iter2 != frame->ml_frames.end(); iter2++)
		{
			AtlasMultiLayerFrame& mlf = *iter2;
			mlf.Frame = unsorted_frames.at(index + mlf.FrameOffset)->frame;
			frame->frame->MultiLayerFrames.push_back(mlf);
		}		
	}

	// Add frame specific animations.
	for (std::vector<Atlas_Build_Anim>::iterator iter = add_anims.begin(); iter != add_anims.end(); iter++)
	{
		Atlas_Build_Anim& anim = *iter;
		std::vector<AtlasFrame*> frames;

		for (std::vector<Atlas_Build_Frame*>::iterator iter = anim.frames.begin(); iter != anim.frames.end(); iter++)
		{
			frames.push_back((*iter)->frame);
		}

		atlas->Add_Animation(anim.name.c_str(), anim.speed, anim.mode, frames);
	}

	// Add each of the animations.
	std::vector<ConfigFileNode> animations = asset_file.Get<std::vector<ConfigFileNode> >("animations/animation");
	for (std::vector<ConfigFileNode>::iterator iter = animations.begin(); iter != animations.end(); iter++)
	{
		ConfigFileNode node = *iter;		

		const char* name = asset_file.Get<const char*>("name", node, true);
		const char* mode = asset_file.Get<const char*>("mode", node, true);
		float	   speed = asset_file.Get<float>("speed", node, true);
		std::vector<AtlasFrame*> out_frames;

		// Decode the animation mode.
		AnimationMode::Type anim_mode = Parse_Animation_Mode(mode);

		// Work out the names of each of the frames.
		std::vector<ConfigFileNode> frames = asset_file.Get<std::vector<ConfigFileNode> >("frame", node);
		for (std::vector<ConfigFileNode>::iterator frameiter = frames.begin(); frameiter != frames.end(); frameiter++)
		{
			ConfigFileNode frame_node = *frameiter;			
			const char*	   frame_type = asset_file.Get<const char*>("type", frame_node, true);

			if (stricmp(frame_type, "single") == 0)
			{
				const char*	frame_name = asset_file.Get<const char*>("name", frame_node, true);
				AtlasFrame* frame = atlas->Get_Frame(frame_name);
				DBG_ASSERT_STR(frame != NULL, "Invalid frame name '%s' in animation '%s'.", frame_name, name);
				out_frames.push_back(frame);
			}
			else if (stricmp(frame_type, "multiple") == 0)
			{
				std::vector<AtlasFrame*>& frames = atlas->Get_Frames_List();

				const char*	frame_name = asset_file.Get<const char*>("name", frame_node, true);
				std::string name_str = frame_name;

				int hash_mark = name_str.find('#');
				DBG_ASSERT_STR(hash_mark != std::string::npos, "Animation marked as type 'multiple' does not contain replacement hash mark.");

				std::string split_left  = name_str.substr(0, hash_mark);
				std::string split_right = name_str.substr(hash_mark + 1);

				for (std::vector<AtlasFrame*>::iterator animiter = frames.begin(); animiter != frames.end(); animiter++)
				{
					AtlasFrame* anim = *animiter;

					if (anim->Name.size() > split_left.size() + split_right.size())
					{
						if (anim->Name.substr(0, split_left.size()) == split_left &&
							anim->Name.substr((anim->Name.size() - split_right.size()), split_right.size()) == split_right)
						{
							AtlasFrame* frame = atlas->Get_Frame(anim->Name.c_str());
							DBG_ASSERT_STR(frame != NULL, "Invalid frame name '%s' in animation '%s'.", anim->Name.c_str(), name);
							out_frames.push_back(frame);
						}
					}
				}
			}
			else if (stricmp(frame_type, "multiple_indexed") == 0)
			{
				std::vector<AtlasFrame*>& frames = atlas->Get_Frames_List();

				int	from_frame = asset_file.Get<int>("from", frame_node, true);
				int	to_frame = asset_file.Get<int>("to", frame_node, true);

				DBG_ASSERT_STR(from_frame < to_frame, "Animation marked as 'multiple_indexed' has an ending index lower than starting index!");
				DBG_ASSERT_STR(from_frame >= 0 && from_frame < (int)frames.size(), "Animation marked as 'multiple_indexed' has an invalid starting index!");
				DBG_ASSERT_STR(to_frame >= 0 && to_frame < (int)frames.size(), "Animation marked as 'multiple_indexed' has an invalid ending index!");

				for (int i = from_frame; i <= to_frame; i++)
				{
					AtlasFrame* anim = atlas->Get_Frame_By_Index(i);
					DBG_ASSERT(anim != NULL);
					out_frames.push_back(anim);
				}
			}
			else
			{
				DBG_ASSERT_STR(false, "Unknown or invalid frame type '%s'", frame_type);
			}
		}

		// Add the animation.
		atlas->Add_Animation(name, speed, anim_mode, out_frames);
	}

	// Save compiled file.
	atlas->Unlock_Textures();

	// Save compiled binary output.
	PackageFile* file = builder->Get_Package_File();
	PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Write);

	int texture_count = 0;
	AtlasTexture** textures = atlas->Get_Textures(texture_count);

	std::vector<AtlasFrame*> frames = atlas->Get_Frames_List();
	std::vector<AtlasAnimation*> anims = atlas->Get_Animations_List();

	std::vector<int> frame_name_pointers;
	std::vector<int> anim_name_pointers;
	std::vector<int> anim_frame_pointers;
	std::vector<int> texture_data_pointers;

	// CompiledAtlasHeader
	int name_ptr = stream->Create_Pointer();
	stream->Write<u32>(StringHelper::Hash(atlas->Get_Name().c_str()));
	stream->Write<u32>(texture_count);
	stream->Write<u32>(frames.size());
	stream->Write<u32>(anims.size());
	int frame_ptr_index = stream->Create_Pointer();
	int animation_ptr_index = stream->Create_Pointer();
	int data_ptr_index = stream->Create_Pointer();
	stream->Align_To_Pointer();

	// Frame data.
	stream->Patch_Pointer(frame_ptr_index);
	std::vector<int> mlf_pointers;
	for (unsigned int i = 0; i < frames.size(); i++)
	{
		AtlasFrame* frame = frames.at(i);

		// CompiledAtlasFrame
		frame_name_pointers.push_back(stream->Create_Pointer());
		stream->Write<u32>(frame->NameHash);
		stream->Write<u32>(frame->TextureIndex);
		stream->Write<u32>(i);
		stream->Write<u32>(frame->HasSemiTransparentPixels ? 1 : 0);
		stream->Write<float>(frame->DepthBias);

			stream->Write<float>(frame->Rect.X);
			stream->Write<float>(frame->Rect.Y);
			stream->Write<float>(frame->Rect.Width);
			stream->Write<float>(frame->Rect.Height);
			stream->Align_To_Pointer();

			stream->Write<float>(frame->UV.X);
			stream->Write<float>(frame->UV.Y);
			stream->Write<float>(frame->UV.Width);
			stream->Write<float>(frame->UV.Height);
			stream->Align_To_Pointer();

			stream->Write<float>(frame->Origin.X);
			stream->Write<float>(frame->Origin.Y);
			stream->Align_To_Pointer();

			stream->Write<float>(frame->GridOrigin.X);
			stream->Write<float>(frame->GridOrigin.Y);
			stream->Write<float>(frame->GridOrigin.Width);
			stream->Write<float>(frame->GridOrigin.Height);
			stream->Align_To_Pointer();

			stream->Write<float>(frame->PixelBounds.X);
			stream->Write<float>(frame->PixelBounds.Y);
			stream->Write<float>(frame->PixelBounds.Width);
			stream->Write<float>(frame->PixelBounds.Height);
			stream->Align_To_Pointer();
		
		// Multilayer frame info.
		stream->Write<int>(frame->MultiLayerFrames.size());
		mlf_pointers.push_back(stream->Create_Pointer());

		stream->Align_To_Pointer();
	}
	
	for (unsigned int i = 0; i < frames.size(); i++)
	{
		AtlasFrame* frame = frames.at(i);
		int mlf_ptr = mlf_pointers.at(i);

		stream->Patch_Pointer(mlf_ptr);
		for (unsigned int j = 0; j < frame->MultiLayerFrames.size(); j++)
		{
			AtlasMultiLayerFrame& mlf_frame = frame->MultiLayerFrames[j];
	
			// CompiledAtlasMultiLayerFrame
			stream->Write<int>(std::find(frames.begin(), frames.end(), mlf_frame.Frame) - frames.begin());
			stream->Write<int>(mlf_frame.FrameOffset);
			stream->Write<int>(mlf_frame.LayerOffset);

			stream->Align_To_Pointer();
		}
	}

	// Animation data.
	stream->Patch_Pointer(animation_ptr_index);
	for (unsigned int i = 0; i < anims.size(); i++)
	{
		AtlasAnimation* anim = anims.at(i);

		// CompiledAtlasAnimation
		anim_name_pointers.push_back(stream->Create_Pointer());
		stream->Write<u32>(anim->NameHash);
		stream->Write<u32>(anim->Mode);
		stream->Write<float>(anim->Speed);
		stream->Write<u32>(anim->Frames.size());
		anim_frame_pointers.push_back(stream->Create_Pointer());
		stream->Align_To_Pointer();
	}

	// Write animation frame data.
	for (unsigned int i = 0; i < anims.size(); i++)
	{
		AtlasAnimation* anim = anims.at(i);
		int anim_ptr_index = anim_frame_pointers.at(i);

		stream->Patch_Pointer(anim_ptr_index);

		// Array of pointers.
		for (std::vector<AtlasFrame*>::iterator iter = anim->Frames.begin(); iter != anim->Frames.end(); iter++)
		{
			AtlasFrame* anim_frame = (*iter);

			// CompiledAtlasFrame
			int ptr_index = stream->Create_Pointer();
			stream->Patch_Pointer(ptr_index, frame_name_pointers.at(anim_frame->Index));
		}
	}

	// Texture data.
	stream->Patch_Pointer(data_ptr_index);
	for (int i = 0; i < texture_count; i++)
	{
		Pixelmap* pixmap = textures[i]->PixelmapPtr;

		// CompiledPixmap
		stream->Write<u32>(pixmap->Get_Width());
		stream->Write<u32>(pixmap->Get_Height());
		stream->Write<u32>(PixelmapFormat::R8G8B8A8);
		texture_data_pointers.push_back(stream->Create_Pointer());
		stream->Align_To_Pointer();
	}

	// Actual compressed texture data.
	//DBG_LOG("Compressing atlas texture data to R8G8B8A8 ...");
	for (int i = 0; i < texture_count; i++)
	{
		Pixelmap* pixmap = textures[i]->PixelmapPtr;
		u8* data = pixmap->Get_Data();

		int data_ptr = texture_data_pointers.at(i);

		//int output_size = squish::GetStorageRequirements(pixmap->Get_Width(), pixmap->Get_Height(), squish::kDxt5);
		int output_size = pixmap->Get_Width() * pixmap->Get_Height() * 4;
		char* output = (char*)pixmap->Get_Data();

		//char* output = new char[output_size];

		//squish::CompressImage(data, pixmap->Get_Width(), pixmap->Get_Height(), output, squish::kDxt5);

		PixelmapFactory::Save(StringHelper::Format("DebugOutput/%s_%i.png", atlas->Get_Name().c_str(), i).c_str(), pixmap);

		stream->Patch_Pointer(data_ptr);
		stream->WriteBuffer(output, 0, output_size);

		//SAFE_DELETE(output);
	}

	// Write strings.
	for (unsigned int i = 0; i < frames.size(); i++)
	{
		AtlasFrame* frame = frames.at(i);
		int name_ptr = frame_name_pointers.at(i);
		stream->Patch_Pointer(name_ptr);
		stream->WriteNullTerminatedString(frame->Name.c_str());
	}
	for (unsigned int i = 0; i < anims.size(); i++)
	{
		AtlasAnimation* anim = anims.at(i);
		int name_ptr = anim_name_pointers.at(i);
		stream->Patch_Pointer(name_ptr);
		stream->WriteNullTerminatedString(anim->Name.c_str());
	}

	// Write name.
	stream->Patch_Pointer(name_ptr);
	stream->WriteNullTerminatedString(atlas->Get_Name().c_str());

	stream->Close();
	SAFE_DELETE(stream);

	// Delete old resources.
	for (std::vector<Pixelmap*>::iterator iter = loaded_pixmaps.begin(); iter != loaded_pixmaps.end(); iter++)
	{
		SAFE_DELETE(*iter);
	}
	loaded_pixmaps.clear();

	for (std::vector<Atlas_Build_Frame*>::iterator iter = add_frames.begin(); iter != add_frames.end(); iter++, index++)
	{
		Atlas_Build_Frame* frame = (*iter);
		SAFE_DELETE(frame);
	}
	add_frames.clear();

//	stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Read);
//	CompiledAtlasHeader* header = reinterpret_cast<CompiledAtlasHeader*>(stream->Get_Data());
//	stream->Close();
//	SAFE_DELETE(stream);

	SAFE_DELETE(atlas);

	return true;
}

void AtlasCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	builder->Get_Package_File()->Add_Chunk(compiled_asset_path.c_str(), asset.Source_Path.c_str(), PackageFileChunkType::Atlas, false,asset.Priority.c_str(), asset.Out_Of_Date);
}