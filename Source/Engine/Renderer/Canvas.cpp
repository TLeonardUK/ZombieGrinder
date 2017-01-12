// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Canvas.h"
#include "Engine/Resources/ResourceFactory.h"

Canvas::Canvas(Rect2D bounds)
	: m_bounds(bounds)
	, m_blend(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha)
	, m_color(Color::White)
	, m_global_color(Color::White)
	, m_scale(1.0f, 1.0f)
	, m_rotation(0)
	, m_viewport(m_bounds)
	, m_font(NULL)
	, m_camera(NULL)
	, m_screen_camera(NULL)
{
}

Canvas::Canvas()
	: m_bounds(Rect2D(0, 0, 0, 0))
	, m_blend(RendererOption::E_Src_Alpha_One_Minus_Src_Alpha)
	, m_color(Color::White)
	, m_global_color(Color::White)
	, m_scale(1.0f, 1.0f)
	, m_rotation(0)
	, m_viewport(m_bounds)
	, m_font(NULL)
	, m_camera(NULL)
	, m_screen_camera(NULL)
{
}

void Canvas::Set_Blend(RendererOption::Type blend)
{
	m_blend = blend;
}

RendererOption::Type Canvas::Get_Blend()
{
	return m_blend;
}

Camera* Canvas::Get_Camera()
{
	return m_camera;
}

void Canvas::Set_Camera(Camera* camera)
{
	m_camera = camera;
}

Camera*	Canvas::Get_Screen_Camera()
{
	return m_screen_camera;
}

void Canvas::Set_Screen_Camera(Camera* camera)
{
	m_screen_camera = camera;
}

void Canvas::Set_Color(Color v)
{
	m_color = v;
}

Color Canvas::Get_Color()
{
	return m_color;
}

void Canvas::Set_Global_Color(Color v)
{
	m_global_color = v;
}

Color Canvas::Get_Global_Color()
{
	return m_global_color;
}

void Canvas::Set_Scale(Vector2 v)
{
	m_scale = v;
}

Vector2 Canvas::Get_Scale()
{
	return m_scale;
}

void Canvas::Set_Rotation(float v)
{
	m_rotation = v;
}

float Canvas::Get_Rotation()
{
	return m_rotation;
}

void Canvas::Set_Bounds(Rect2D bounds)
{
	m_bounds = bounds;
	m_viewport = m_viewport.ClampInside(bounds);
}

Rect2D Canvas::Get_Bounds()
{
	return m_bounds;
}

void Canvas::Set_Viewport(Rect2D view)
{
	m_viewport = view.ClampInside(m_bounds);
}

Rect2D Canvas::Get_Viewport()
{
	return m_viewport;
}

void Canvas::Set_Font(FontHandle* view)
{
	m_font = view;
	m_font_renderer = FontRenderer(m_font, false, true);
	m_markup_font_renderer = MarkupFontRenderer(m_font, false, true, false);
}

FontHandle* Canvas::Get_Font()
{
	return m_font;
}

bool Canvas::Get_Font_Shadowed()
{
	return m_font_renderer.Get_Shadow();
}

void Canvas::Set_Font_Shadowed(bool value)
{
	m_font_renderer.Set_Shadow(value);
	m_markup_font_renderer.Set_Shadow(value);
}

Rect2D Canvas::World_To_Screen(Rect2D world_bb)
{
	Camera* active_cam = Get_Camera();
	Camera* screen_cam = Get_Screen_Camera();
	Rect2D screen_bb = active_cam->Get_Screen_Viewport();

	if (active_cam == NULL || screen_cam == NULL)
	{
		return Rect2D(world_bb.X, world_bb.Y, world_bb.Width, world_bb.Height);
	}

	Vector2 top_left = active_cam->Project(Vector2(world_bb.X, world_bb.Y), screen_bb);
	Vector2 bottom_right = active_cam->Project(Vector2(world_bb.X + world_bb.Width, world_bb.Y + world_bb.Height), screen_bb);

	Rect2D screen_bbox = Rect2D
		(
			top_left.X,
			top_left.Y,
			bottom_right.X - top_left.X,
			bottom_right.Y - top_left.Y
		);

	return screen_bbox;
}

Rect2D Canvas::Screen_To_World(Rect2D scrren_bb)
{
	Camera* active_cam = Get_Camera();
	Camera* screen_cam = Get_Screen_Camera();
	Rect2D screen_bb = active_cam->Get_Screen_Viewport();

	if (active_cam == NULL || screen_cam == NULL)
	{
		return Rect2D(scrren_bb.X, scrren_bb.Y, scrren_bb.Width, scrren_bb.Height);
	}

	Rect2D cam_screen_box = active_cam->Get_Screen_Viewport();
	Rect2D cam_world_box = active_cam->Get_Bounding_Viewport();

	Rect2D world_bbox = Rect2D
	(
		cam_world_box.X + ((scrren_bb.X / cam_screen_box.Width) * cam_world_box.Width),
		cam_world_box.Y + ((scrren_bb.Y / cam_screen_box.Height) * cam_world_box.Height),
		(scrren_bb.Width / cam_screen_box.Width) * cam_world_box.Width,
		(scrren_bb.Height / cam_screen_box.Height) * cam_world_box.Height
	);

	return world_bbox;
}

void Canvas::Draw_Frame(const char* frame_name, Vector2 position, bool flip_h, bool flip_v)
{
	m_atlas_renderer.Draw_Frame(
		frame_name, 
		position,
		0.0f,
		m_color * m_global_color,
		flip_h,
		flip_v,
		m_blend,
		m_scale,
		m_rotation);
}

void Canvas::Draw_Frame(AtlasFrame* frame, Vector2 position, bool flip_h, bool flip_v)
{
	m_atlas_renderer.Draw_Frame(
		frame, 
		position,
		0.0f,
		m_color * m_global_color,
		flip_h,
		flip_v,
		m_blend,
		m_scale,
		m_rotation);
}

void Canvas::Draw_Frame(const char* frame_name, Rect2D position, bool flip_h, bool flip_v)
{
	m_atlas_renderer.Draw_Frame(
		frame_name, 
		position,
		0.0f,
		m_color * m_global_color,
		flip_h,
		flip_v,
		m_blend,
		m_scale,
		m_rotation);
}

void Canvas::Draw_Animation(const char* frame_name, Rect2D position, bool flip_h, bool flip_v)
{
	AtlasAnimation* f = ResourceFactory::Get()->Get_Atlas_Animation(frame_name);
	if (f == NULL)
	{
		DBG_LOG("Attempt to draw invalid animation frame: %s", frame_name);
		return;
	}

	m_atlas_renderer.Draw_Frame(
		f->Frames[0], 
		position,
		0.0f,
		m_color * m_global_color,
		flip_h,
		flip_v,
		m_blend,
		m_scale,
		m_rotation);
}

void Canvas::Draw_Frame(AtlasFrame* frame, Rect2D position, bool flip_h, bool flip_v)
{
	m_atlas_renderer.Draw_Frame(
		frame, 
		position,
		0.0f,
		m_color * m_global_color,
		flip_h,
		flip_v,
		m_blend,
		m_scale,
		m_rotation);
}

void Canvas::Draw_Frame_Box(const char* frame_name, Rect2D bounds, float scale)
{
	UIFrame(frame_name).Draw_Frame(m_atlas_renderer, bounds, Vector2(m_scale.X * scale, m_scale.Y * scale), m_color * m_global_color);
}

void Canvas::Draw_Frame_Box(UIFrame frame, Rect2D bounds)
{
	frame.Draw_Frame(m_atlas_renderer, bounds, m_scale, m_color * m_global_color);
}

void Canvas::Draw_Rect(Rect2D bounds)
{
	m_prim_renderer.Draw_Solid_Quad(bounds, m_color * m_global_color);
}

void Canvas::Draw_Hollow_Rect(Rect2D bounds, float line_width)
{
	m_prim_renderer.Draw_Wireframe_Quad(bounds, 0.0f, m_color * m_global_color, line_width);
}

void Canvas::Draw_Oval(Rect2D bounds)
{
	m_prim_renderer.Draw_Solid_Oval(bounds, m_color * m_global_color);
}

void Canvas::Draw_Line(Vector2 start, Vector2 end, float line_width)
{
	m_prim_renderer.Draw_Line(
		Vector3(start.X, start.Y, 0), 
		Vector3(end.X, end.Y, 0), 
		line_width, 
		m_color * m_global_color);
}

void Canvas::Draw_Text(const char* text, Rect2D bounds, float font_height, TextAlignment::Type h_align, TextAlignment::Type v_align)
{
	m_font_renderer.Draw_String(text, bounds, font_height, m_color * m_global_color, h_align, v_align, m_scale, m_blend);
}

void Canvas::Draw_Text(const char* text, Vector2 bounds, float font_height)
{
	m_font_renderer.Draw_String(text, bounds, font_height, m_color * m_global_color, Vector2(0, 0), m_scale, m_blend);
}

Vector2 Canvas::Calculate_Text_Size(const char* text, float font_height)
{
	return m_font_renderer.Calculate_String_Size(text, font_height);
}

std::string Canvas::Word_Wrap(const char* text, Rect2D bounds, float font_height)
{
	return m_font_renderer.Word_Wrap(text, bounds, font_height);
}
	
void Canvas::Draw_Markup_Text(const char* text, Rect2D bounds, float font_height, TextAlignment::Type h_align, TextAlignment::Type v_align)
{
	m_markup_font_renderer.Draw_String(text, bounds, font_height, m_color * m_global_color, h_align, v_align, m_scale, m_blend);
}

void Canvas::Draw_Markup_Text(const char* text, Vector2 bounds, float font_height)
{
	m_markup_font_renderer.Draw_String(text, bounds, font_height, m_color * m_global_color, Vector2(0, 0), m_scale, m_blend);
}

Vector2 Canvas::Calculate_Markup_Text_Size(const char* text, float font_height)
{
	return m_markup_font_renderer.Calculate_String_Size(text, font_height);
}

std::string Canvas::Word_Wrap_Markup(const char* text, Rect2D bounds, float font_height)
{
	return m_markup_font_renderer.Word_Wrap(text, bounds, font_height);
}

std::string Canvas::Escape_Markup(const char* text)
{
	return MarkupFontRenderer::Escape(text);
}