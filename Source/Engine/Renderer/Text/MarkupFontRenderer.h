// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RENDERER_TEXT_MARKERFONTRENDERER_
#define _ENGINE_RENDERER_TEXT_MARKERFONTRENDERER_

#include "Generic/Types/LinkedList.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Types/Vector2.h"
#include "Generic/Types/Variant.h"
#include "Engine/IO/Stream.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Renderer/Renderer.h"

#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontHandle.h"
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/FontRenderer.h"

#include "Engine/Renderer/Shaders/ShaderFactory.h"
#include "Engine/Renderer/Shaders/ShaderProgram.h"

#include "Generic/Types/Color.h"

struct MarkupTokenType
{
	enum Type
	{
		Text,
		Tag,
		End
	};
};

struct MarkupToken
{
	MEMORY_ALLOCATOR(MarkupToken, "Rendering");

public:
	MarkupTokenType::Type Type;
	std::string			  Value;
	std::vector<Variant>  Parameters;
};

class MarkupParser
{
	MEMORY_ALLOCATOR(MarkupParser, "Rendering");

private:
	const char* m_text;
	int			m_offset;
	int			m_length;

	void Parse_Tag(MarkupToken& token);

public:
	void Start(const char* text);
	void Next(MarkupToken& token);

};

class MarkupFontRenderer 
{
	MEMORY_ALLOCATOR(MarkupFontRenderer, "Rendering");

private:
	FontHandle*				m_font;
	FontRenderer			m_font_renderer;

	bool					m_persist_state;
	
	// State stack.
	int						m_bold_depth;
	std::vector<Color>		m_color_stack;
	std::vector<double>		m_fade_stack;

public:

	// Constructors
	MarkupFontRenderer();	
	MarkupFontRenderer(FontHandle* font, bool buffered = false, bool shadow = true, bool persist_state = false);	
	~MarkupFontRenderer();	

	// Rendering methods.
	void Set_Shadow(bool value) { m_font_renderer.Set_Shadow(value); }

	FontRenderState Begin_Draw(Vector2 pen, bool do_not_draw = false) { return m_font_renderer.Begin_Draw(pen, do_not_draw); }

	void Draw(
		FontRenderState& state, 
		const char* text, 
		Vector2 location, 
		float font_height, 
		Color color = Color::White, 
		Vector2 extra_spacing = Vector2(0, 0), 
		Vector2 scale = Vector2(1.0f, 1.0f), 
		RendererOption::Type blend = RendererOption::E_Src_Alpha_One_Minus_Src_Alpha);

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
		
	void Clear_State();

	Vector2 Calculate_String_Size(const char* text, float font_height, Vector2 scale = Vector2(1.0f, 1.0f));
	std::string Word_Wrap(const char* text, Rect2D bounds, float font_height, Vector2 scale = Vector2(1.0f, 1.0f));

	static std::string Escape(std::string value);

};

#endif

