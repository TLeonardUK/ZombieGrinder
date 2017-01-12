// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FREETYPE_FONT_
#define _ENGINE_RENDERER_TEXT_FREETYPE_FONT_

#include "Generic/Types/Rect2D.h"
#include "Generic/Math/RectanglePacker.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Engine/Resources/Compiled/Fonts/CompiledFont.h"

#include "Generic/Types/Vector2.h"
#include "Generic/Types/HashTable.h"

#include <ft2build.h>
#include FT_FREETYPE_H

class ConfigFile;
class Pixelmap;

#define SDF_DOWNSCALE

struct FreeType_Face
{
	MEMORY_ALLOCATOR(FreeType_Face, "Rendering");

public:
	FT_Face			Face;
	int				BufferSize;
	char*			Buffer;

};


struct FreeType_FontGlyph
{
	MEMORY_ALLOCATOR(FreeType_FontGlyph, "Rendering");

public:
	unsigned int	FreeType_GlyphIndex;
	int				TextureIndex;
	FontGlyph		Glyph;
};

struct FreeType_FontTexture
{
	MEMORY_ALLOCATOR(FreeType_FontTexture, "Rendering");

public:
	Texture*		TexturePtr;
	Pixelmap*		PixelmapPtr;
	bool			Is_Dirty;
	RectanglePacker	Packer;
};

class FreeType_Font : public Font
{
	MEMORY_ALLOCATOR(FreeType_Font, "Rendering");

private:
	FT_Library										m_library;
	std::vector<FreeType_Face>						m_faces;

	FreeType_FontTexture**							m_textures;
	HashTable<FreeType_FontGlyph*, unsigned int>	m_glyphs;
	int												m_current_dirty_texture_index;

	int												m_texture_size;
	int												m_max_textures;
	int												m_glyph_size;
	int												m_glyph_spacing;
	float											m_shadow_scale;

	bool											m_textures_locked;
	
	Vector2											m_glyph_bounds;
	Vector2											m_baseline;

	std::string										m_name;

	void*											m_resource_data;

protected:

	void Set_Name(const char* name);

	void Get_FT_Glyph_MinMax(unsigned int glyph, Vector2& min, Vector2& max);

	FT_Face Get_Face_For_Glyph(unsigned int glyph);

public:

	// Destructor!
	FreeType_Font();
	FreeType_Font(FT_Library library, std::vector<FreeType_Face> faces, ConfigFile* config, std::vector<u32> glyphs);
	FreeType_Font(FT_Library library, std::vector<FreeType_Face> faces);
	~FreeType_Font();

	// FreeType stuff.
	void Lock_Textures();
	FreeType_FontTexture** Get_Textures(int& texture_count);
	void Unlock_Textures();
	void Add_Glyphs(std::vector<u32> glyphs);
	void Add_Glyph(unsigned int glyph);
	HashTable<FreeType_FontGlyph*, unsigned int>& Get_Glyphs();
	bool Load_Compiled_Config(CompiledFontHeader* config);

	// Actual font stuff!	
	FontGlyph Get_Glyph(unsigned int character);
	Vector2	  Get_Kerning(unsigned int prev, unsigned int next);
	std::string Get_Name();
	float Get_Base_Height();
	float Get_Shadow_Scale();

	Vector2		Get_Glyph_Bounds();
	Vector2		Get_Glyph_Baseline();

};

#endif

