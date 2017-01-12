// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_COMPILED_FONTS_
#define _ENGINE_RESOURCES_COMPILED_FONTS_

#include "Engine/Renderer/Text/Font.h"

#include "Engine/Resources/Compiled/CompiledPixelmap.h"

struct CompiledFontGlyph
{
	u32		texture_index;
	u32		ft_glyph_index;
	u32		glyph;
	float	glyph_advance;
	Vector2 glyph_size;
	Rect2D  glyph_uv;
};

struct CompiledFontFace
{
	int			size;
	const char*	buffer;
};

struct CompiledFontHeader
{
	const char*			name;
	u32					texture_count;
	u32					max_texture_count;
	u32					texture_size;
	u32					glyph_spacing;
	u32					glyph_count;
	u32					glyph_size;
	u32					glyph_bounds_x;
	u32					glyph_bounds_y;
	u32					glyph_baseline_x;
	u32					glyph_baseline_y;
	float				shadow_scale;
	u32					face_count;
	CompiledPixelmap*	textures;
	CompiledFontGlyph*	glyphs;
	CompiledFontFace*	faces;
};

#endif