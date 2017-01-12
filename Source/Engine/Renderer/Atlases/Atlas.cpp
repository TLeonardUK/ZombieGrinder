// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/Atlas.h"

#include "Engine/Scene/Animation.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include <algorithm>

Atlas::Atlas(ConfigFile* config)
	: m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_resource_data(NULL)
{
	// Load configuration.
	m_texture_size  = config->Get<int>("settings/texture-size");
	m_max_textures	= config->Get<int>("settings/max-textures");
	m_name			= config->Get<const char*>("settings/name");
	
	// Create texture buffer.
	m_textures = new AtlasTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	
}

Atlas::Atlas()
	: m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_resource_data(NULL)
{
}

Atlas::~Atlas()
{
	for (HashTable<AtlasFrame*, unsigned int>::Iterator iter = m_frames.Begin(); iter != m_frames.End(); iter++)
	{
		delete *iter;
	}

	for (HashTable<AtlasAnimation*, unsigned int>::Iterator iter = m_animations.Begin(); iter != m_animations.End(); iter++)
	{
		delete *iter;
	}

	for (int i = 0; i < m_max_textures; i++)
	{
		AtlasTexture* texture = m_textures[i];
		if (texture != NULL)
		{
			if (texture->PixelmapPtr != NULL)
			{
				SAFE_DELETE(texture->PixelmapPtr);
			}
			if (texture->TexturePtr != NULL)
			{
				SAFE_DELETE(texture->TexturePtr);
			}
			SAFE_DELETE(texture);
		}
	}

	SAFE_DELETE(m_textures);
	SAFE_DELETE(m_resource_data);

	m_frames.Clear();
	m_frames_list.clear();
	m_animations.Clear();
	m_animations_by_start_frame.Clear();
}

void Atlas::Lock_Textures()
{
	m_textures_locked = true;
}

AtlasTexture** Atlas::Get_Textures(int& texture_count)
{
	texture_count = m_current_dirty_texture_index + 1;
	return m_textures;
}

void Atlas::Unlock_Textures()
{
	m_textures_locked = false;

	for (int i = 0; i < m_max_textures; i++)
	{
		AtlasTexture* texture = m_textures[i];
		if (texture != NULL && texture->Is_Dirty == true)
		{
			if (texture->TexturePtr != NULL)
			{
				texture->TexturePtr->Set_Pixelmap(texture->PixelmapPtr);
			}
			texture->Is_Dirty = false;
		}
	}
 }

AtlasTexture* Atlas::Get_Texture(int index)
{
	DBG_ASSERT(index >= 0 && index < m_max_textures);
	return m_textures[index];
}

HashTable<AtlasFrame*, unsigned int>& Atlas::Get_Frames()
{
	return m_frames;
}

std::vector<AtlasFrame*>& Atlas::Get_Frames_List()
{
	return m_frames_list;
}

HashTable<AtlasAnimation*, unsigned int>& Atlas::Get_Animations()
{
	return m_animations;
}

std::vector<AtlasAnimation*>& Atlas::Get_Animations_List()
{
	return m_animations_list;
}

int Atlas::Get_Frame_Index(AtlasFrame* frame)
{
	std::vector<AtlasFrame*>::iterator iter = std::find(m_frames_list.begin(), m_frames_list.end(), frame);
	if (iter == m_frames_list.end())
	{
		return -1;
	}
	else
	{
		return std::distance(m_frames_list.begin(), iter);
	}
}

AtlasFrame* Atlas::Get_Frame(const char* name)
{
	return m_frames.Get(StringHelper::Hash(name));
}

AtlasFrame* Atlas::Get_Frame(unsigned int name_hash)
{
	return m_frames.Get(name_hash);
}

AtlasFrame* Atlas::Get_Frame_By_Index(int index)
{
	if (index < 0 || index >= (int)m_frames_list.size())
	{
		return NULL;
	}
	return m_frames_list.at(index);
}

AtlasAnimation* Atlas::Get_Animation_By_Start_Frame(int frame)
{
	return m_animations_by_start_frame.Get(frame);
}

int	Atlas::Get_Frame_Count()
{
	return m_frames_list.size();
}

void Atlas::Add_Frame(const char* name, Pixelmap* source_pixmap, Rect2D texture_window, Vector2 origin, float depth_bias, Rect2D grid_origin)
{
	Renderer*			renderer		= Renderer::Try_Get();
	bool				new_texture		= false;
	Rect2D				output_rect;
	int					source_width	= source_pixmap->Get_Width();
	int					source_height	= source_pixmap->Get_Height();
	unsigned char*		source_data		= reinterpret_cast<unsigned char*>(source_pixmap->Get_Data());

	if (m_current_dirty_texture_index < 0)
	{
		new_texture = true;
	}
	else
	{
		new_texture = !m_textures[m_current_dirty_texture_index]->Packer.Pack(Vector2(texture_window.Width, texture_window.Height), output_rect);
	}

	if (new_texture == true)
	{	
		DBG_ASSERT_STR(++m_current_dirty_texture_index < m_max_textures, "Ran out of atlas texture space.");

		m_textures[m_current_dirty_texture_index]				= new AtlasTexture();
		m_textures[m_current_dirty_texture_index]->Packer		= RectanglePacker(m_texture_size, m_texture_size, 3);
		m_textures[m_current_dirty_texture_index]->PixelmapPtr	= new Pixelmap(m_texture_size, m_texture_size, PixelmapFormat::R8G8B8A8);
		m_textures[m_current_dirty_texture_index]->PixelmapPtr->Clear(Color(0, 0, 0, 0));

		if (renderer != NULL)
		{
			m_textures[m_current_dirty_texture_index]->TexturePtr = renderer->Create_Texture(m_textures[m_current_dirty_texture_index]->PixelmapPtr, TextureFlags::NONE);
		}
		else
		{
			m_textures[m_current_dirty_texture_index]->TexturePtr = NULL;
		}

		bool packed = m_textures[m_current_dirty_texture_index]->Packer.Pack(Vector2(texture_window.Width, texture_window.Height), output_rect);
		DBG_ASSERT_STR(packed, "Could not pack frame into brand new texture! To large?");
	}

	// Work out glyph texture position etc.
	float half_u = (1.0f / float(m_texture_size)) * 0.5f;
	float half_v = (1.0f / float(m_texture_size)) * 0.5f;

	int pixel_x	= (int)output_rect.X;
	int pixel_y	= (int)output_rect.Y;
	int pixel_w	= (int)output_rect.Width;
	int pixel_h	= (int)output_rect.Height;		
	float uv_w	= ((output_rect.Width) / float(m_texture_size));		
	float uv_h	= ((output_rect.Height) / float(m_texture_size)) ;
	float uv_x	= ((output_rect.X) / float(m_texture_size)) ;
	float uv_y	= ((output_rect.Y) / float(m_texture_size)) ;
	
	// Origins that are fractional are considered a fraction
	// of the source texture size (0.5 = 50% etc)
	if (origin.X >= 0 && origin.X <= 1)
	{
		origin.X = pixel_w * origin.X;
	}
	if (origin.Y >= 0 && origin.Y <= 1)
	{
		origin.Y = pixel_h * origin.Y;
	}

	// Only render if we have a bitmap for this glyph.
	AtlasTexture* texture = m_textures[m_current_dirty_texture_index];

	// Copy pixels to output.
	PixelmapWindow window = source_pixmap->Window(Rect2D(texture_window.X, texture_window.Y, output_rect.Width, output_rect.Height));

	DBG_ASSERT(	pixel_x >= 0 && 
				pixel_y >= 0 &&
				pixel_x + pixel_w < m_texture_size &&
				pixel_y + pixel_h < m_texture_size);

	texture->PixelmapPtr->Paste(Vector2(output_rect.X, output_rect.Y), window, 1);

	// Calculate bounds.
	Rect2D bounds = window.Calculate_Bounds();

	//DBG_LOG("Frame[%s] %f,%f,%f,%f", name, bounds.X, bounds.Y, bounds.Width, bounds.Height);

	// Add glyph to list.	
	AtlasFrame* frame		= new AtlasFrame();
	frame->Name				= name;
	frame->NameHash			= StringHelper::Hash(name);
	frame->TexturePtr		= m_textures[m_current_dirty_texture_index];
	frame->TextureIndex		= m_current_dirty_texture_index;
	frame->UV				= Rect2D(uv_x, uv_y, uv_w, uv_h);
	frame->Rect				= Rect2D(pixel_x, pixel_y, pixel_w, pixel_h);
	frame->Origin			= origin;
	frame->Index			= m_frames_list.size();
	frame->AtlasPtr			= this;
	frame->GridOrigin		= grid_origin;
	frame->PixelBounds		= bounds;
	frame->DepthBias		= depth_bias;

	// Calculate if we have semi-transparent pixels or not.
	frame->HasSemiTransparentPixels	= false;

	for (int y = 0; y < output_rect.Height && frame->HasSemiTransparentPixels == false; y++)
	{
		for (int x = 0; x < output_rect.Width && frame->HasSemiTransparentPixels == false; x++)
		{
			Color c = window.Get_Pixel(x, y);
			if (c.A < 255 && c.A > 0)
			{
				frame->HasSemiTransparentPixels = true;
			}
		}
	}

	m_textures[m_current_dirty_texture_index]->Is_Dirty = true;

	DBG_ASSERT_STR(!m_frames.Contains(frame->NameHash), "Atlas already contains frame with name '%s'", frame->Name.c_str());

	m_frames.Set(frame->NameHash, frame);
	m_frames_list.push_back(frame);
}

void Atlas::Add_Animation(const char* name, float speed, AnimationMode::Type mode, std::vector<AtlasFrame*> frames)
{
	AtlasAnimation* anim	= new AtlasAnimation();
	anim->Name				= name;
	anim->NameHash			= StringHelper::Hash(name);
	anim->Mode				= mode;
	anim->Speed				= speed;
	anim->Frames			= frames;
	anim->AtlasPtr				= this;
	
	DBG_ASSERT_STR(!m_animations.Contains(anim->NameHash), "Atlas already contains animation with name '%s'", anim->Name.c_str());

	m_animations.Set(anim->NameHash, anim);	

	int idx = Get_Frame_Index(anim->Frames.at(0));
	if (!m_animations_by_start_frame.Contains(idx))
	{
		m_animations_by_start_frame.Set(idx, anim);
	}
	m_animations_list.push_back(anim);
}

void Atlas::Set_Name(const char* name)
{
	m_name = name;
}

std::string Atlas::Get_Name()
{
	return m_name;
}

bool Atlas::Load_Compiled_Config(CompiledAtlasHeader* config)
{	
	m_resource_data = config;

	// Reset configuration to null (we shouldn't be modifying this atlas if we have loaded from compiled config).
	m_texture_size  = config->pixmaps[0].width;
	m_max_textures	= config->texture_count;
	m_name			= config->name;

	// Create texture buffer.
	m_textures = new AtlasTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Load in all textures.
	for (unsigned int i = 0; i < config->texture_count; i++)
	{
		m_current_dirty_texture_index++;

		m_textures[m_current_dirty_texture_index]				= new AtlasTexture();
		m_textures[m_current_dirty_texture_index]->Is_Dirty		= false;
		m_textures[m_current_dirty_texture_index]->PixelmapPtr	= new Pixelmap(&config->pixmaps[i]);

		TextureFlags::Type flags = TextureFlags::NONE;

		if (*EngineOptions::generate_item_schema)
		{
			flags = TextureFlags::PersistSourceData;
		}

		m_textures[m_current_dirty_texture_index]->TexturePtr	= Renderer::Get()->Create_Texture(m_textures[m_current_dirty_texture_index]->PixelmapPtr, flags);
		m_textures[m_current_dirty_texture_index]->Packer		= RectanglePacker(0, 0);		
	}

	// We don't want to try appending glyphs to last loaded texture, so move to the "next" one.
	m_current_dirty_texture_index++;

	// Load all frames.
	for (unsigned int i = 0; i < config->frame_count; i++)
	{
		CompiledAtlasFrame compiled_frame = config->frames[i];

		// Add frame to list.	
		AtlasFrame* frame		= new AtlasFrame();	
		frame->Name				= compiled_frame.name;
		frame->NameHash			= compiled_frame.name_hash;
		frame->Rect				= compiled_frame.rect;
		frame->TexturePtr		= m_textures[compiled_frame.texture_index];
		frame->TextureIndex		= compiled_frame.texture_index;
		frame->UV				= compiled_frame.uv;
		frame->Origin			= compiled_frame.origin;
		frame->DepthBias		= compiled_frame.depth_bias;
		frame->GridOrigin		= compiled_frame.grid_origin;
		frame->PixelBounds		= compiled_frame.pixel_bounds;
		frame->HasSemiTransparentPixels	= (compiled_frame.has_semitransparent_pixels == 1);
		frame->Index			= m_frames_list.size();
		frame->AtlasPtr			= this;

		m_frames.Set(frame->NameHash, frame);
		m_frames_list.push_back(frame);
	}

	for (unsigned int i = 0; i < config->frame_count; i++)
	{
		CompiledAtlasFrame compiled_frame = config->frames[i];
		AtlasFrame* frame = m_frames_list.at(i);

		// Load multi layer frames.
		for (int j = 0; j < compiled_frame.multi_layer_frame_count; j++)
		{
			CompiledAtlasMultiLayerFrame* ml_frame = compiled_frame.multi_layer_frames + j;
			
			AtlasMultiLayerFrame m;
			m.Frame = m_frames_list.at(ml_frame->frame_index);
			m.FrameOffset = ml_frame->frame_offset;
			m.LayerOffset = ml_frame->layer_offset;

			frame->MultiLayerFrames.push_back(m);
		}
	}

	// Load all animations.
	for (unsigned int i = 0; i < config->animation_count; i++)
	{
		CompiledAtlasAnimation animation = config->animations[i];

		// Add animation to list.	
		AtlasAnimation* anim	= new AtlasAnimation();	
		anim->Name				= animation.name;
		anim->NameHash			= animation.name_hash;
		anim->Speed				= animation.speed;
		anim->Mode				= animation.mode;
		anim->AtlasPtr			= this;

		// Load frames.
		for (unsigned int j = 0; j < animation.frame_count; j++)
		{
			CompiledAtlasFrame* frame = animation.frames[j];
			anim->Frames.push_back(m_frames_list.at(frame->index));
		}

		m_animations.Set(anim->NameHash, anim);

		int start_index = anim->Frames.at(0)->Index;

		if (!m_animations_by_start_frame.Contains(start_index))
			m_animations_by_start_frame.Set(start_index, anim);

		m_animations_list.push_back(anim);
	}

	// Free resource data, we don't care about it anymore.
	SAFE_DELETE(m_resource_data);

	return true;
}