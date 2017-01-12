// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FreeType/FreeType_Font.h"
#include "Engine/Renderer/Text/FreeType/FreeType_FontFactory.h"

#include "Engine/Renderer/Textures/TextureFactory.h"
#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Generic/Math/Math.h"

#include "Engine/Localise/Locale.h"

FreeType_Font::FreeType_Font(FT_Library library, std::vector<FreeType_Face> faces, ConfigFile* config, std::vector<u32> glyphs)
	: m_library(library)
	, m_faces(faces)
	, m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_name("")
	, m_resource_data(NULL)
{
	// Load configuration.
	m_texture_size  = config->Get<int>("generation/texture-size");
	m_max_textures	= config->Get<int>("generation/max-textures");
	m_glyph_size    = config->Get<int>("generation/glyph-size");
	m_glyph_spacing = config->Get<int>("generation/glyph-spacing");
	m_shadow_scale  = config->Get<float>("generation/shadow-scale");
	m_name			= config->Get<const char*>("generation/name");
	
	// Create texture buffer.
	m_textures = new FreeType_FontTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Set face to use our character size.
	int result = 0;

	for (int i = 0; i < (int)faces.size(); i++)
	{
		result = FT_Set_Pixel_Sizes(faces[i].Face, m_glyph_size, m_glyph_size);
		DBG_ASSERT_STR(result == 0, "Failed to set character size of freetype font.");
	}

	// Work out max glyph height and use that for our all glyphs.
	Vector2 global_min, global_max;
	for (int i = 0; i < (int)glyphs.size(); i++)
	{
		unsigned int glyph = glyphs[i]; 

		Vector2 min, max;
		Get_FT_Glyph_MinMax(glyph, min, max);
		
		if (min.X < global_min.X)
			global_min.X = min.X;
		if (min.Y < global_min.Y)
			global_min.Y = min.Y;
		if (max.X > global_max.X)
			global_max.X = max.X;
		if (max.Y > global_max.Y)
			global_max.Y = max.Y;
	}

	// Normalize to 0-x range.
	global_max.X -= global_min.X;
	global_max.Y -= global_min.Y;

	global_min.X = -global_min.X;
	global_min.Y = -global_min.Y;

	m_glyph_bounds = Vector2(Max(global_max.X, global_max.Y), Max(global_max.X, global_max.Y));
	m_baseline = global_min;
	
	//DBG_LOG("Glyph Bounds = %f,%f", global_max.X, global_max.Y);
	//DBG_LOG("Baseline     = %f,%f", m_baseline.X, m_baseline.Y);

	// Create texture.
	Lock_Textures();
	Add_Glyphs(glyphs);
	Unlock_Textures();
}

FreeType_Font::FreeType_Font(FT_Library library, std::vector<FreeType_Face> faces)
	: m_library(library)
	, m_faces(faces)
	, m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_resource_data(NULL)
{
}

FreeType_Font::FreeType_Font()
	: m_library(NULL)
	, m_textures_locked(false)
	, m_current_dirty_texture_index(-1)
	, m_resource_data(NULL)
{
}

FreeType_Font::~FreeType_Font()
{
	for (int i = 0; i < m_max_textures; i++)
	{
		FreeType_FontTexture* texture = m_textures[i];
		if (texture != NULL)
		{
			SAFE_DELETE(texture);
		}
	}

	SAFE_DELETE(m_resource_data);
	SAFE_DELETE(m_textures);

	for (int i = 0; i < (int)m_faces.size(); i++)
	{
		FT_Done_Face(m_faces[i].Face);
		SAFE_DELETE_ARRAY(m_faces[i].Buffer);
	}

	m_library = NULL;
}

void FreeType_Font::Set_Name(const char* name)
{
	m_name = name;
}

std::string FreeType_Font::Get_Name()
{
	return m_name;
}

float FreeType_Font::Get_Base_Height()
{
	return (float)m_glyph_size;
}

float FreeType_Font::Get_Shadow_Scale()
{
	return (float)m_shadow_scale;
}

Vector2	FreeType_Font::Get_Glyph_Bounds()
{
	return m_glyph_bounds;
}

Vector2	FreeType_Font::Get_Glyph_Baseline()
{
	return m_baseline;
}

bool FreeType_Font::Load_Compiled_Config(CompiledFontHeader* config)
{	
	m_resource_data = config;

	// Load configuration.
	m_max_textures	= config->max_texture_count;
	m_texture_size  = config->texture_size;
	m_glyph_spacing = config->glyph_spacing;
	m_name			= config->name;
	m_glyph_size	= config->glyph_size;
	m_shadow_scale	= config->shadow_scale;
	m_glyph_bounds	= Vector2((float)config->glyph_bounds_x, (float)config->glyph_bounds_y);
	m_baseline		= Vector2((float)config->glyph_baseline_x, (float)config->glyph_baseline_y);

	m_library		= FreeType_FontFactory::Get_FreeType_Library();

	// Create texture buffer.
	m_textures = new FreeType_FontTexture*[m_max_textures];
	memset(m_textures, 0, m_max_textures * sizeof(Texture*));	

	// Load in all textures.
	for (unsigned int i = 0; i < config->texture_count; i++)
	{
		m_current_dirty_texture_index++;

		m_textures[m_current_dirty_texture_index]				= new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Is_Dirty		= false;
		// If we are dedicated we will never render anyway, so skip!
		m_textures[m_current_dirty_texture_index]->PixelmapPtr	= NULL;
		m_textures[m_current_dirty_texture_index]->PixelmapPtr	= new Pixelmap(&config->textures[i]);
		m_textures[m_current_dirty_texture_index]->TexturePtr	= Renderer::Get()->Create_Texture(m_textures[m_current_dirty_texture_index]->PixelmapPtr, TextureFlags::NONE);
		m_textures[m_current_dirty_texture_index]->Packer		= RectanglePacker(0, 0, m_glyph_spacing);		
	}

	// We don't want to try appending glyphs to last loaded texture, so move to the "next" one.
	m_current_dirty_texture_index++;

	// Load all glyphs.
	for (unsigned int i = 0; i < config->glyph_count; i++)
	{
		CompiledFontGlyph& glyph = config->glyphs[i];

		FreeType_FontGlyph* new_glyph	= new FreeType_FontGlyph();	
		new_glyph->FreeType_GlyphIndex	= glyph.ft_glyph_index;
		new_glyph->Glyph.TexturePtr		= m_textures[glyph.texture_index]->TexturePtr;		
		new_glyph->Glyph.Glyph			= glyph.glyph;
		new_glyph->Glyph.UV				= glyph.glyph_uv;
		new_glyph->TextureIndex			= glyph.texture_index;
		new_glyph->Glyph.Size			= glyph.glyph_size;
		new_glyph->Glyph.Advance		= glyph.glyph_advance;
		
		m_glyphs.Set(new_glyph->Glyph.Glyph, new_glyph);
	}

	// Load in all faces incase we want to do dynamic creation.
#ifndef DEDICATED_SERVER_BUILD // Waste of memory in dedicated builds, we will never be rendering anyway.
	for (unsigned int i = 0; i < config->face_count; i++)
	{
		CompiledFontFace& face = config->faces[i];

		// Load in font face.
		FT_Face ftface;

		int result = FT_New_Memory_Face(m_library, (FT_Byte*)face.buffer, face.size, 0, &ftface);
		if (result != 0)
		{
			DBG_LOG("Failed to open freetype font from compiled resource.");
			continue;
		}

		result = FT_Set_Pixel_Sizes(ftface, m_glyph_size, m_glyph_size);
		DBG_ASSERT_STR(result == 0, "Failed to set character size of freetype font.");
	
		// Store face.
		FreeType_Face f;
		f.Face = ftface;
		f.Buffer = NULL; // Don't want to keep a reference to buffer, as we don't want to delete it as its part of resource data.
		f.BufferSize = 0;
		m_faces.push_back(f);
	}
#endif

	return true;
}

FontGlyph FreeType_Font::Get_Glyph(unsigned int glyph) 
{
	FreeType_FontGlyph* output;
	if (m_glyphs.Get(glyph, output))
	{
		return output->Glyph;
	}
	else
	{
		Lock_Textures();
		Add_Glyph(glyph);
		Unlock_Textures();

		/*
		if (m_glyphs.Get('?', output))
		{
			return output->Glyph;
		}
		else
		{
			DBG_ASSERT(false);
		}
		*/
	}

	return FontGlyph();
}

void FreeType_Font::Lock_Textures()
{
	m_textures_locked = true;
}

void FreeType_Font::Unlock_Textures()
{
	m_textures_locked = false;

	for (int i = 0; i < m_max_textures; i++)
	{
		FreeType_FontTexture* texture = m_textures[i];
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

FreeType_FontTexture** FreeType_Font::Get_Textures(int& texture_count)
{
	texture_count = m_current_dirty_texture_index + 1;
	return m_textures;
}

void FreeType_Font::Add_Glyphs(std::vector<u32> glyphs)
{
	DBG_ASSERT(m_textures_locked == true);

	//double time_accumulator = 0.0f;
	//double time_counter = 0;

	for (int i = 0; i < (int)glyphs.size(); i++)
	{
		unsigned int glyph = glyphs[i]; 

		//double time = Platform::Get()->Get_Ticks();

		Add_Glyph(glyph);

		//time_accumulator += (Platform::Get()->Get_Ticks() - time);
		//time_counter++;

		//double time_per_glyph = time_accumulator / time_counter;
		//double minutes_left = ((time_per_glyph * (glyphs.size() - i)) / 1000) / 60;

		//DBG_LOG("Added glyph %i of %i (Estimated %.2fm remaining).", i, length, minutes_left);
	}
}

Vector2 FreeType_Font::Get_Kerning(unsigned int prev, unsigned int next)
{
	FreeType_FontGlyph* prev_glyph = m_glyphs.Get(prev);
	FreeType_FontGlyph* next_glyph = m_glyphs.Get(next);

	FT_Vector kerning;

	FT_Face face = Get_Face_For_Glyph(prev);

	int result = FT_Get_Kerning(face, prev_glyph->FreeType_GlyphIndex, next_glyph->FreeType_GlyphIndex, FT_KERNING_DEFAULT, &kerning);
	DBG_ASSERT(result == 0);

	return Vector2((float)kerning.x, (float)kerning.y);
}

HashTable<FreeType_FontGlyph*, unsigned int>& FreeType_Font::Get_Glyphs()
{
	return m_glyphs;
}

void FreeType_Font::Get_FT_Glyph_MinMax(unsigned int glyph, Vector2& min, Vector2& max)
{
	FT_Face face = Get_Face_For_Glyph(glyph);

	int ft_glyph_index = FT_Get_Char_Index(face, glyph);
	if (ft_glyph_index <= 0)
	{
		DBG_LOG("Undefined character 0x%08x in font.", glyph);
		ft_glyph_index = FT_Get_Char_Index(face, '?');	
		DBG_ASSERT_STR(ft_glyph_index > 0, "No question mark in font!", glyph);
	}

	int result = FT_Load_Glyph(face, ft_glyph_index, FT_LOAD_RENDER);
	DBG_ASSERT_STR(result == 0, "Unable to load character 0x%08x from freetype.", ft_glyph_index);

	min.X = (float)face->glyph->bitmap_left;
	max.X = (float)face->glyph->bitmap_left + face->glyph->bitmap.width;
	min.Y = (float)-face->glyph->bitmap_top;
	max.Y = (float)face->glyph->bitmap.rows - face->glyph->bitmap_top;
}

FT_Face FreeType_Font::Get_Face_For_Glyph(unsigned int glyph)
{
	for (std::vector<FreeType_Face>::iterator iter = m_faces.begin(); iter != m_faces.end(); iter++)
	{
		FreeType_Face& face = *iter;

		int ft_glyph_index = FT_Get_Char_Index(face.Face, glyph);
		if (ft_glyph_index > 0)
		{
			return face.Face;
		}
	}

	// Fallback to first face and use whatever invalid glyph we require.
	return m_faces[0].Face;
}

void FreeType_Font::Add_Glyph(unsigned int glyph)
{
	Renderer* renderer = Renderer::Try_Get();

	// Check it dosen't already exist.
	if (m_glyphs.Contains(glyph))
	{
		return;
	}

	FT_Face face = Get_Face_For_Glyph(glyph);

	// Generate glyph with freetype.
	int ft_glyph_index = FT_Get_Char_Index(face, glyph);
	if (ft_glyph_index <= 0)
	{
		DBG_LOG("Undefined character 0x%08x in font.", glyph);

		// Use a question mark.
		ft_glyph_index = FT_Get_Char_Index(face, '?');
	
		DBG_ASSERT_STR(ft_glyph_index > 0, "No question mark in font!", glyph);
	}

	// Load in the glyph.
	int result = FT_Load_Glyph(face, ft_glyph_index, FT_LOAD_RENDER);
	DBG_ASSERT_STR(result == 0, "Unable to load character 0x%08x from freetype.", ft_glyph_index);

	// Work out position on texture.
	bool  new_texture = false;
	Rect2D  glyph_rect = Rect2D(0, 0, 0, 0);

	if (m_current_dirty_texture_index < 0)
	{
		new_texture = true;
	}
	else
	{
		new_texture = m_textures[m_current_dirty_texture_index] == NULL || !m_textures[m_current_dirty_texture_index]->Packer.Pack(m_glyph_bounds, glyph_rect);
	}

	if (new_texture == true)
	{	
		DBG_ASSERT_STR(++m_current_dirty_texture_index < m_max_textures, "Ran out of font texture space.");

		m_textures[m_current_dirty_texture_index] = new FreeType_FontTexture();
		m_textures[m_current_dirty_texture_index]->Packer = RectanglePacker(m_texture_size, m_texture_size, m_glyph_spacing);
		m_textures[m_current_dirty_texture_index]->PixelmapPtr = new Pixelmap(m_texture_size, m_texture_size, PixelmapFormat::R8G8B8A8);
		m_textures[m_current_dirty_texture_index]->PixelmapPtr->Clear(Color(0, 0, 0, 0));

		if (renderer != NULL)
		{
			m_textures[m_current_dirty_texture_index]->TexturePtr = renderer->Create_Texture(m_textures[m_current_dirty_texture_index]->PixelmapPtr, (TextureFlags::Type)(TextureFlags::LinearFilter|TextureFlags::PersistSourceData));
		}
		else
		{
			m_textures[m_current_dirty_texture_index]->TexturePtr = NULL;
		}

		bool packed = m_textures[m_current_dirty_texture_index]->Packer.Pack(m_glyph_bounds, glyph_rect);
		DBG_ASSERT_STR(packed, "Could not pack glyph into brand new texture! To large?");
	}

	// Get glyph Pixelmap.
	Pixelmap* glyph_Pixelmap = new Pixelmap((unsigned char*)face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, face->glyph->bitmap.width, PixelmapFormat::R8, false);
	
	// Work out glyph texture position etc.
	int pixel_x	= (int)glyph_rect.X;
	int pixel_y	= (int)glyph_rect.Y;
	int pixel_w	= (int)glyph_rect.Width;
	int pixel_h	= (int)glyph_rect.Height;
		
	float uv_x	= (glyph_rect.X) / float(m_texture_size);
	float uv_y	= (glyph_rect.Y) / float(m_texture_size);
	float uv_w	= (glyph_rect.Width) / float(m_texture_size);		
	float uv_h	= (glyph_rect.Height) / float(m_texture_size);

	float advance_offset = 0;

	// Only render if we have a bitmap for this glyph.
	if (face->glyph->bitmap.buffer != NULL)
	{
		FreeType_FontTexture* texture = m_textures[m_current_dirty_texture_index];
		
		Vector2 glyph_min, glyph_max;	
		glyph_min.X = (float)face->glyph->bitmap_left;
		glyph_max.X = (float)face->glyph->bitmap_left + face->glyph->bitmap.width;
		glyph_min.Y = (float)-face->glyph->bitmap_top;
		glyph_max.Y = (float)face->glyph->bitmap.rows - face->glyph->bitmap_top;

		advance_offset = glyph_min.X;

		int offset_pixel_x = (int)((pixel_x + glyph_min.X) + m_baseline.X);
		int offset_pixel_y = (int)((pixel_y + glyph_min.Y) + m_baseline.Y);

		DBG_ASSERT((offset_pixel_x - pixel_x) >= 0);
		DBG_ASSERT((offset_pixel_y - pixel_y) >= 0);
		
		//DBG_LOG("%c = %f (%i,%i)", glyph, advance_offset, offset_pixel_x - pixel_x, offset_pixel_y - pixel_y);

		PixelmapWindow window = glyph_Pixelmap->Window(Rect2D(0.0f, 0.0f, (float)face->glyph->bitmap.width, (float)face->glyph->bitmap.rows), false, true);							
		texture->PixelmapPtr->Paste(Vector2(offset_pixel_x, offset_pixel_y), window);
		
		// Mask out transparent pixels.
		for (int y = pixel_y; y < pixel_y + pixel_h; y++)
		{
			for (int x = pixel_x; x < pixel_x + pixel_w; x++)
			{
				Color pc = texture->PixelmapPtr->Get_Pixel(x, y);
				if (pc.R == 0)
				{
					pc = Color(0, 0, 0, 0);
					
				//pc = Color(0, 0, 0, 255);
					//if (x - pixel_x == (m_face->glyph->advance.x >> 6) + advance_offset)
					//{
				//	pc = Color(255, 0, 255, 255);
				//}
				}
				else
				{
					pc = Color(255, 255, 255, 255);
				}
				texture->PixelmapPtr->Set_Pixel(x, y, pc);
			}
		}
	}

	// Add glyph to list.	
	FreeType_FontGlyph* new_glyph = new FreeType_FontGlyph();	
	new_glyph->FreeType_GlyphIndex = ft_glyph_index;
	new_glyph->Glyph.TexturePtr	= m_textures[m_current_dirty_texture_index]->TexturePtr;		
	new_glyph->Glyph.Glyph		= glyph;
	new_glyph->Glyph.UV			= Rect2D(uv_x, uv_y, uv_w, uv_h);
	new_glyph->TextureIndex		= m_current_dirty_texture_index;
	new_glyph->Glyph.Size		= m_glyph_bounds;
	new_glyph->Glyph.Advance	= (float)(face->glyph->advance.x >> 6) + advance_offset; // this not going to work is it? -_-
	
	m_textures[m_current_dirty_texture_index]->Is_Dirty = true;

	m_glyphs.Set(glyph, new_glyph);

	// Release Pixelmap.
	SAFE_DELETE(glyph_Pixelmap);
}
