// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_CANVAS_
#define _ENGINE_CANVAS_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Patterns/Singleton.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Scene/Camera.h"

#include "Engine/UI/UIFrame.h"

#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Renderer/Text/FontRenderer.h"
#include "Engine/Renderer/Text/MarkupFontRenderer.h"

// The canvas class is a wrapper class that provides a monolithic 
// interface for 2D drawing funcitonality. Primary use is interacting
// with scripts.
//
// Really need to refactor the fuck out of the rendering code. So much 
// functionality duplication.
class Canvas
{
	MEMORY_ALLOCATOR(Canvas, "Rendering");

protected:
	RendererOption::Type	m_blend;
	Color					m_color;
	Color					m_global_color;
	Vector2					m_scale;
	float					m_rotation;
	Rect2D					m_bounds;
	Rect2D					m_viewport;
	FontHandle*				m_font;
	AtlasRenderer			m_atlas_renderer;
	PrimitiveRenderer		m_prim_renderer;
	FontRenderer			m_font_renderer;
	MarkupFontRenderer		m_markup_font_renderer;

	Camera*					m_camera;
	Camera*					m_screen_camera;

public:
	Canvas();
	Canvas(Rect2D bounds);

	// State functions.
	void				 Set_Blend(RendererOption::Type blend);
	RendererOption::Type Get_Blend();
	void		Set_Color(Color v);
	Color		Get_Color();
	void		Set_Global_Color(Color v);
	Color		Get_Global_Color();
	void		Set_Scale(Vector2 v);
	Vector2		Get_Scale();
	void		Set_Rotation(float v);
	float		Get_Rotation();
	void		Set_Bounds(Rect2D bounds);
	Rect2D		Get_Bounds();
	void		Set_Viewport(Rect2D view);
	Rect2D		Get_Viewport();
	void		Set_Font(FontHandle* font);
	FontHandle*	Get_Font();
	bool		Get_Font_Shadowed();
	void		Set_Font_Shadowed(bool value);

	Camera*		Get_Camera();
	void		Set_Camera(Camera* camera);

	Camera*		Get_Screen_Camera();
	void		Set_Screen_Camera(Camera* camera);

	Rect2D		World_To_Screen(Rect2D world_bb);
	Rect2D		Screen_To_World(Rect2D world_bb);

	// Atlas drawing.
	void Draw_Animation(const char* frame_name, Rect2D position, bool flip_h = false, bool flip_v = false);
	void Draw_Frame(const char* frame_name, Vector2 position, bool flip_h = false, bool flip_v = false);
	void Draw_Frame(AtlasFrame* frame, Vector2 position, bool flip_h = false, bool flip_v = false);
	void Draw_Frame(const char* frame_name, Rect2D position, bool flip_h = false, bool flip_v = false);
	void Draw_Frame(AtlasFrame* frame, Rect2D position, bool flip_h = false, bool flip_v = false);
	void Draw_Frame_Box(const char* frame_name, Rect2D bounds, float scale = 1.0f);
	void Draw_Frame_Box(UIFrame frame, Rect2D bounds);

	// Primitives.
	void Draw_Rect(Rect2D bounds);
	void Draw_Hollow_Rect(Rect2D bounds, float line_width = 1.0f);
	void Draw_Oval(Rect2D bounds);
	void Draw_Line(Vector2 start, Vector2 end, float line_width = 1.0f);

	// Text.
	void Draw_Text(const char* text, Rect2D bounds, float font_height, TextAlignment::Type h_align = TextAlignment::Left, TextAlignment::Type v_align = TextAlignment::Top);
	void Draw_Text(const char* text, Vector2 bounds, float font_height);
	Vector2 Calculate_Text_Size(const char* text, float font_height);
	std::string Word_Wrap(const char* text, Rect2D bounds, float font_height);

	// Markup
	void Draw_Markup_Text(const char* text, Rect2D bounds, float font_height, TextAlignment::Type h_align = TextAlignment::Left, TextAlignment::Type v_align = TextAlignment::Top);
	void Draw_Markup_Text(const char* text, Vector2 bounds, float font_height);
	Vector2 Calculate_Markup_Text_Size(const char* text, float font_height);
	std::string Word_Wrap_Markup(const char* text, Rect2D bounds, float font_height);
	std::string Escape_Markup(const char* text);

};

#endif

