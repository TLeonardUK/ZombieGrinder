// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_FONTRENDERER_
#define _ENGINE_RENDERER_TEXT_FONTRENDERER_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"
#include "Engine/IO/Stream.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontFactory.h"

#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Generic/Types/Color.h"

struct TextAlignment
{
	enum Type
	{
		Left,
		Right,
		Center,
		Top,
		Bottom,
		Justified,
		ScaleToFit
	};
};

struct FontRenderState
{
	MEMORY_ALLOCATOR(FontRenderState, "Rendering");

public:
	Vector2 Pen;
	Vector2 Pen_Start;
	Vector2 Max_Size;
	bool DoNotDraw;
};

class FontRenderer 
{
	MEMORY_ALLOCATOR(FontRenderer, "Rendering");

private:
	FontHandle*				m_font;

	bool					m_shadow;

	bool					m_buffered;
	RenderTarget*			m_buffer_target;
	Texture*				m_buffer_texture;
	char*					m_buffer_text;
	char*					m_buffer_text_length;

public:

	// Constructors
	FontRenderer();	
	FontRenderer(FontHandle* font, bool buffered = false, bool shadow = true);	
	~FontRenderer();	

	// State-based rendering.
	void Set_Shadow(bool value) { m_shadow = value; }
	bool Get_Shadow() { return m_shadow; }
	
	FontRenderState Begin_Draw(Vector2 pen, bool do_not_draw = false);

	void Draw(
		FontRenderState& state, 
		const char* text,
		float font_height, 
		Color color, 
		Vector2 extra_spacing, 
		Vector2 scale, 
		RendererOption::Type blend);

	// Rendering methods.
	void Draw_String(
		const char* text, 
		Vector2 location, 
		float font_height, 
		Color color = Color::White, 
		Vector2 extra_spacing = Vector2(0, 0), 
		Vector2 scale = Vector2(1.0f, 1.0f), 
		RendererOption::Type blend = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

	void Draw_String(
		const char* text, 
		Rect2D bounds, 
		float font_height, 
		Color color = Color::White, 
		TextAlignment::Type horizontal_align = TextAlignment::Left, 
		TextAlignment::Type vertical_align = TextAlignment::Top, 
		Vector2 scale = Vector2(1.0f, 1.0f), 
		RendererOption::Type blend = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);
	
	Vector2 Calculate_String_Size(const char* text, float font_height, Vector2 scale = Vector2(1.0f, 1.0f));
	std::string Word_Wrap(const char* text, Rect2D bounds, float font_height, Vector2 scale = Vector2(1.0f, 1.0f));

};

#endif

