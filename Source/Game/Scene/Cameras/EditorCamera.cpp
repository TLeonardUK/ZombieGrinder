// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scene/Cameras/EditorCamera.h"
#include "Engine/Display/GfxDisplay.h"
#include "Engine/Input/Input.h"
#include "Generic/Math/Math.h"
#include "Engine/Scene/Map/Map.h"

#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"

#include "Game/Runner/Game.h"

#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIManager.h"

#define ZOOM_THRESHOLD 0.1f

EditorCamera::EditorCamera(float fov, Rect2D viewport)
	: m_zoom(1.0f)
	, m_old_mouse_position(0.0f, 0.0f)
{
	m_fov				= fov;
	m_viewport			= viewport;
	m_input_viewport	= viewport;
	m_near_clip			= 0.0f;
	m_far_clip			= (MAP_LAYER_DEPTH + 1) * (DEFAULT_MAP_DEPTH + 1);
	m_zoom				= 1.0f;
}

void EditorCamera::Set_Input_Viewport(Rect2D rect)
{
	m_input_viewport = rect;
}

Rect2D EditorCamera::Get_Input_Viewport()
{
	return m_input_viewport;
}

Matrix4 EditorCamera::Get_Projection_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Orthographic(0, viewport.Width * m_zoom, viewport.Height * m_zoom, 0, near_clip, far_clip);
}

Matrix4 EditorCamera::Get_View_Matrix()
{
	Rect2D  viewport	= Get_Viewport();
	float near_clip	= Get_Near_Clip();
	float far_clip	= Get_Far_Clip();

	return Matrix4::Translate(-m_position.Floor());
}

Rect2D EditorCamera::Get_Viewport()
{
	return Rect2D
	(
		m_viewport.X,
		m_viewport.Y,
		m_viewport.Width,// * m_zoom,
		m_viewport.Height//, * m_zoom
	);
}

Rect2D EditorCamera::Get_Bounding_Viewport()
{
	return Rect2D
	(
		m_position.X, 
		m_position.Y, 
		m_viewport.Width * m_zoom,//(1.0f / m_zoom), // Well that sounds wrong. 
		m_viewport.Height * m_zoom//(1.0f / m_zoom)
	);
}

float EditorCamera::Get_Zoom()
{
	return m_zoom;
}

void EditorCamera::Set_Zoom(float value)
{
	m_zoom = value;
}

void EditorCamera::Tick(const FrameTime& time)
{
	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_position =  mouse->Get_Position();

	if (m_input_viewport.Intersects(mouse_position))
	{
		// Zoom in and out with mouse wheel.
		float wheel_value = -mouse->Get_Scroll_Value();
		if (wheel_value != 0.0f)
		{
			Vector2 old_viewport = Vector2(m_viewport.Width * m_zoom, m_viewport.Height * m_zoom);
			m_zoom = Max(ZOOM_THRESHOLD, m_zoom + (wheel_value * ZOOM_THRESHOLD));
			Vector2 new_viewport = Vector2(m_viewport.Width * m_zoom, m_viewport.Height * m_zoom);
		
			Vector2 relative_mouse_position = mouse_position - Vector2(m_input_viewport.X, m_input_viewport.Y);

			Vector2 mouse_offset = Vector2(relative_mouse_position.X / m_viewport.Width, relative_mouse_position.Y / m_viewport.Height);

			// Reposition camera to keep centered.
			Vector2 diff = Vector2(old_viewport.X - new_viewport.X, old_viewport.Y - new_viewport.Y);
			m_position.X += diff.X * mouse_offset.X;
			m_position.Y += diff.Y * mouse_offset.Y;
		}

		// Move based on mouse delta.
		if (mouse->Is_Button_Down(InputBindings::Mouse_Right))
		{
			float scalar = m_zoom;
			Vector2 delta = m_old_mouse_position - mouse_position;

			m_position.X += delta.X * scalar;
			m_position.Y += delta.Y * scalar;
		}
	}

	m_old_mouse_position = mouse_position;
}