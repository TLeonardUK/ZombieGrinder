// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Builder/Compilers/Fonts/FontCompiler.h"
#include "Builder/Builder/AssetBuilder.h"

#include "Engine/Renderer/Text/FreeType/FreeType_Font.h"
#include "Engine/Renderer/Text/FreeType/FreeType_FontFactory.h"

#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "libsquish/squish.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"

#include <algorithm>

int FontCompiler::Get_Version()
{
	return 40;
}

std::vector<AssetToCompile> FontCompiler::Gather_Assets(AssetBuilder* builder)
{
	std::vector<AssetToCompile> result;

	ConfigFile* file = builder->Get_Package_Config_File();

	if (!file->Contains("fonts"))
	{
		return result;
	}

	std::vector<ConfigFileNode> atlases	= file->Get<std::vector<ConfigFileNode> >("fonts/font");

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
			DBG_LOG("[WARNING] Failed to check dependencies for atlas. Bank file does not exist! '%s'.", asset.Source_Path.c_str());
			continue;
		}

		std::vector<FreeType_Face> faces;
		std::vector<ConfigFileNode> ttf_nodes = asset_file.Get<std::vector<ConfigFileNode> >("generation/ttfs/ttf");
		for (std::vector<ConfigFileNode>::iterator iter = ttf_nodes.begin(); iter != ttf_nodes.end(); iter++)
		{
			ConfigFileNode& node = *iter;

			std::string ttf_path = node->value();
			asset.Dependent_Files.push_back(ttf_path);
		}

		// Add language files as dependencies.
		std::vector<ConfigFileNode> languages	= file->Get<std::vector<ConfigFileNode> >("languages/language");
		for (std::vector<ConfigFileNode>::iterator atlas_iter = languages.begin(); atlas_iter != languages.end(); atlas_iter++)
		{
			ConfigFileNode& node = *atlas_iter;
			const char* path = file->Get<const char*>("file", node, true);
			asset.Dependent_Files.push_back(path);
		}

		result.push_back(asset);
	}

	return result;
}

bool FontCompiler::Build_Asset(AssetBuilder* builder, AssetToCompile asset)
{
	DBG_LOG("Compiling font resource '%s'.", asset.Source_Path.c_str());

	FT_Library library = FreeType_FontFactory::Get_FreeType_Library();

	// Calculate all dependencies.
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	// Load asset file.
	ConfigFile asset_file;
	if (!asset_file.Load(resolved_asset_path.c_str()))
	{
		return false;
	}

	int max_textures = asset_file.Get<int>("generation/max-textures");
	int texture_size  = asset_file.Get<int>("generation/texture-size");
	int glyph_spacing = asset_file.Get<int>("generation/glyph-spacing");

	// Load all glyphs from all language files.
	ConfigFile* package_file = builder->Get_Package_Config_File();
	std::vector<ConfigFileNode> atlases	= package_file->Get<std::vector<ConfigFileNode> >("languages/language");
	std::vector<u32> language_glyphs;
	for (std::vector<ConfigFileNode>::iterator atlas_iter = atlases.begin(); atlas_iter != atlases.end(); atlas_iter++)
	{
		ConfigFileNode& node = *atlas_iter;

		const char* path = package_file->Get<const char*>("file", node, true);
		std::string resolved_language_path = builder->Resolve_Asset_Path(path);
		ConfigFile language_file;

		language_file.Set_Root_Element("resources");

		if (!language_file.Load(resolved_language_path.c_str()))
		{
			return false;
		}
		else
		{	
			std::vector<ConfigFileNode> nodes = language_file.Get<std::vector<ConfigFileNode> >("string");
			for (std::vector<ConfigFileNode>::iterator glyph_iter = nodes.begin(); glyph_iter != nodes.end(); glyph_iter++)
			{
				std::vector<u32> split_glyphs;
				ConfigFileNode node = *glyph_iter;
				StringHelper::Split_UTF8(node->value(), split_glyphs);

				for (int i = 0; i < (int)split_glyphs.size(); i++)
				{
					u32 g = split_glyphs[i];
					if (std::find(language_glyphs.begin(), language_glyphs.end(), g) == language_glyphs.end())
					{
						language_glyphs.push_back(g);
					}
				}
			}
		}
	}

	// Open the ttf file.
	std::vector<FreeType_Face> faces;
	std::vector<ConfigFileNode> ttf_nodes = asset_file.Get<std::vector<ConfigFileNode> >("generation/ttfs/ttf");
	for (std::vector<ConfigFileNode>::iterator iter = ttf_nodes.begin(); iter != ttf_nodes.end(); iter++)
	{
		ConfigFileNode& node = *iter;

		std::string ttf_path = node->value();
		std::string resolved_ttf_path = builder->Resolve_Asset_Path(ttf_path.c_str());

		Stream* ttf_stream = StreamFactory::Open(resolved_ttf_path.c_str(), StreamMode::Read);
		if (ttf_stream == NULL)
		{
			DBG_LOG("Failed to compile font, ttf file could not be found: '%s'", ttf_path.c_str());
			return false;
		}

		// Load in the font file.
		int size = ttf_stream->Length();
		char* buffer = new char[size];
		ttf_stream->ReadBuffer(buffer, 0, size);

		// Load in font face.
		FT_Face face;

		int result = FT_New_Memory_Face(library, (FT_Byte*)buffer, size, 0, &face);
		if (result != 0)
		{
			DBG_LOG("Failed to open freetype font face: %s", ttf_path.c_str());
			SAFE_DELETE(buffer);
			SAFE_DELETE(ttf_stream);
			return NULL;
		}

		// Store face.
		FreeType_Face ftface;
		ftface.Face = face;
		ftface.Buffer = buffer;
		ftface.BufferSize = size;
		faces.push_back(ftface);

		// Return resulting font!
		SAFE_DELETE(ttf_stream);
	}

	// Load into a font class.
	FreeType_Font* font = new FreeType_Font(library, faces, &asset_file, language_glyphs);

	// Save compiled binary output.
	PackageFile* file = builder->Get_Package_File();
	PatchedBinaryStream* stream = new PatchedBinaryStream(compiled_asset_path.c_str(), StreamMode::Write);

	int texture_count = 0;
	FreeType_FontTexture** textures = font->Get_Textures(texture_count);

	HashTable<FreeType_FontGlyph*, unsigned int> glyphs = font->Get_Glyphs();

	std::vector<int> texture_data_pointers;
	std::vector<int> face_data_pointers;

	// CompiledFontHeader
	int name_ptr_index = stream->Create_Pointer();
	stream->Write<u32>(texture_count);
	stream->Write<u32>(max_textures);
	stream->Write<u32>(texture_size);
	stream->Write<u32>(glyph_spacing);
	stream->Write<u32>(glyphs.Size());
	stream->Write<u32>((u32)font->Get_Base_Height());
	stream->Write<u32>((u32)font->Get_Glyph_Bounds().X);
	stream->Write<u32>((u32)font->Get_Glyph_Bounds().Y);
	stream->Write<u32>((u32)font->Get_Glyph_Baseline().X);
	stream->Write<u32>((u32)font->Get_Glyph_Baseline().Y);
	stream->Write<float>(font->Get_Shadow_Scale());
	stream->Write<u32>(faces.size());
	int data_ptr_index = stream->Create_Pointer();
	int glyph_ptr_index = stream->Create_Pointer();
	int faces_ptr_index = stream->Create_Pointer();
	stream->Align_To_Pointer();

	// Glyph information.
	stream->Patch_Pointer(glyph_ptr_index);
	for (HashTable<FreeType_FontGlyph*, unsigned int>::Iterator iter = glyphs.Begin(); iter != glyphs.End(); iter++)
	{
		FreeType_FontGlyph* glyph = *iter;

		// CompiledFontGlyph
		stream->Write<u32>(glyph->TextureIndex);
		stream->Write<u32>(glyph->FreeType_GlyphIndex);
		stream->Write<u32>(glyph->Glyph.Glyph);

			stream->Write<float>(glyph->Glyph.Advance);
			stream->Align_To_Pointer();

			stream->Write<float>(glyph->Glyph.Size.X);
			stream->Write<float>(glyph->Glyph.Size.Y);
			stream->Align_To_Pointer();

			stream->Write<float>(glyph->Glyph.UV.X);
			stream->Write<float>(glyph->Glyph.UV.Y);
			stream->Write<float>(glyph->Glyph.UV.Width);
			stream->Write<float>(glyph->Glyph.UV.Height);
			stream->Align_To_Pointer();

		stream->Align_To_Pointer();
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
	DBG_LOG("Compressing atlas texture data to R8G8B8A8 ...");
	for (int i = 0; i < texture_count; i++)
	{
		Pixelmap* pixmap = textures[i]->PixelmapPtr;
		u8* data = pixmap->Get_Data();

		//PixelmapFactory::Save(StringHelper::Format("Font_%i.png", i).c_str(), pixmap);

		int data_ptr = texture_data_pointers.at(i);

		int output_size = pixmap->Get_Width() * pixmap->Get_Height() * 4;
		char* output = (char*)pixmap->Get_Data();
		
		stream->Patch_Pointer(data_ptr);
		stream->WriteBuffer(output, 0, output_size);
	}

	// Faces data.
	stream->Patch_Pointer(faces_ptr_index);
	for (int i = 0; i < (int)faces.size(); i++)
	{
		FreeType_Face& face = faces[i];

		// CompiledFontFace
		stream->Write<u32>(face.BufferSize);
		face_data_pointers.push_back(stream->Create_Pointer());

		stream->Align_To_Pointer();
	}

	// Face data.
	for (int i = 0; i < (int)faces.size(); i++)
	{
		FreeType_Face& face = faces[i];

		stream->Patch_Pointer(face_data_pointers[i]);
		stream->WriteBuffer(face.Buffer, 0, face.BufferSize);
	}

	// Write strings.
	stream->Patch_Pointer(name_ptr_index);
	stream->WriteNullTerminatedString(font->Get_Name().c_str());

	stream->Close();
	SAFE_DELETE(stream);
	SAFE_DELETE(font);

	return true;
}

void FontCompiler::Add_Chunks(AssetBuilder* builder, AssetToCompile asset)
{
	std::string resolved_asset_path = builder->Resolve_Asset_Path(asset.Source_Path);
	std::string compiled_asset_path = builder->Get_Compiled_Asset_File(asset.Source_Path);

	builder->Get_Package_File()->Add_Chunk(compiled_asset_path.c_str(), asset.Source_Path.c_str(), PackageFileChunkType::Font, false, asset.Priority.c_str(), asset.Out_Of_Date);
}