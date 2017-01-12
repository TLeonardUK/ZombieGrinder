// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/UI/Scenes/Editor/EditorTileSelection.h"

#include "Game/Runner/Game.h"
#include "Game/Scene/Cameras/EditorCamera.h"

#include "Engine/Renderer/PrimitiveRenderer.h"
#include "Engine/Renderer/RenderPipeline.h"

#include "Engine/Engine/GameEngine.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Scene/Map/Map.h"

#include "Engine/Input/Input.h"

EditorTileSelection::EditorTileSelection()
{
}

EditorTileSelection::EditorTileSelection(EditorCamera* camera, Vector2 boundry_size)
	: m_is_selecting(false)
	, m_selection_in_progress(false)
	, m_camera(camera)
	, m_boundry_size(boundry_size)
	, m_dragging(false)
	, m_pixel_based(false)
	, m_trigger_timer(0.0f)
{
}

bool EditorTileSelection::Is_Selecting()
{
	return m_is_selecting;
}

Rect2D EditorTileSelection::Get_Selection()
{
	return m_selection_rect;
}

void EditorTileSelection::Clear()
{
	m_is_selecting = false;
	m_selection_in_progress = false;
}

Vector2 EditorTileSelection::Get_Tile_Size()
{
	Map* map = Game::Get()->Get_Map();
	return Vector2(map->Get_Tile_Width(), map->Get_Tile_Height());
}

float EditorTileSelection::Get_Zoom()
{
	EditorCamera* editor_camera = static_cast<EditorCamera*>(Game::Get()->Get_Camera(CameraID::Editor_Main));
	return editor_camera->Get_Zoom();
}

Vector2 EditorTileSelection::Screen_Position_To_Tile_Position(Vector2 position)
{
	GfxDisplay* display = GfxDisplay::Get();

	Rect2D viewport = m_camera->Get_Viewport();

	position.X -= viewport.X;
	position.Y -= viewport.Y;

	float width = viewport.Width;//(float)display->Get_Width();
	float height = viewport.Height;//(float)display->Get_Height();

	Vector4 screen_space_position = Vector4
	(
		((position.X / (float)width) * 2.0f) - 1.0f, 
		((1.0f - (position.Y / (float)height)) * 2.0f) - 1.0f, 
		1.0f, 
		1.0f
	);

	Matrix4 screen_to_map_matrix = (m_camera->Get_Projection_Matrix() * m_camera->Get_View_Matrix()).Inverse();
	Vector4 map_position = screen_to_map_matrix * screen_space_position;

	return Vector2(map_position.X, map_position.Y);
}

Vector2 EditorTileSelection::Get_Cursor_Tile_Position(bool pixel_based)
{
	Input* input = Input::Get();
	MouseState* mouse_state	= input->Get_Mouse_State();

	Vector2 pos = mouse_state->Get_Position();
	pos = Screen_Position_To_Tile_Position(pos);

	Vector2 tile_size = Get_Tile_Size();

	if (pixel_based)
	{
		return Vector2
		(
			pos.X,
			pos.Y
		);
	}
	else
	{
		return Vector2
		(
			floorf(pos.X / tile_size.X),
			floorf(pos.Y / tile_size.Y)
		);
	}
}

bool EditorTileSelection::Tile_Position_In_Boundry(Vector2 position, bool pixel_based)
{
	Vector2 boundry = m_boundry_size;
	if (pixel_based == true)
	{
		return true;
//		boundry.X *= Get_Tile_Size().X;
//		boundry.Y *= Get_Tile_Size().Y;
	}

	return (position.X >= 0 && position.X < boundry.X &&
			position.Y >= 0 && position.Y < boundry.Y);
}

bool EditorTileSelection::Update(bool persist, bool expandable, bool pixel_based, bool continual)
{
	Input* input = Input::Get();
	MouseState* mouse = input->Get_Mouse_State();
	bool result = false;
	
	if (expandable)
	{
		bool mouse_down = mouse->Is_Button_Down(InputBindings::Mouse_Left);
		Vector2 tile_position = Get_Cursor_Tile_Position(pixel_based);

		if (m_dragging == true)
		{
			if (!mouse_down)
			{
				m_dragging = false;
			}
			else
			{
				Vector2 diff = tile_position - m_drag_start;
				m_drag_start = tile_position;

				m_selection_start = m_selection_start + diff;
				m_selection_end = m_selection_end + diff;
			}
		}
		else if (m_selection_in_progress == true)
		{	
			m_selection_end = tile_position;//.MaxValue(m_selection_start);

			if (!mouse_down)
			{
				m_is_selecting = persist;
				m_selection_in_progress = false;
				result = true;
			}
		}
		else
		{
			if (mouse_down && Tile_Position_In_Boundry(tile_position, pixel_based) && m_camera->Get_Input_Viewport().Intersects(mouse->Get_Position()))
			{
				// If clicking inside existing selection, we are dragging selection.
				if (m_is_selecting == true && m_selection_rect.Intersects(tile_position))
				{
					m_dragging = true;
					m_drag_start = tile_position;
				}
				else
				{
					m_selection_in_progress = true;
					m_is_selecting = true;
					m_selection_start = tile_position;
					m_selection_end = tile_position;
				}
			}
		}
	}
	else
	{
		m_is_selecting = false;
		m_selection_in_progress = false;
		
		Vector2 tile_position = Get_Cursor_Tile_Position(pixel_based);

		bool trigger = mouse->Was_Button_Clicked(InputBindings::Mouse_Left);
		if (continual && mouse->Was_Button_Down(InputBindings::Mouse_Left))
		{
			m_trigger_timer += GameEngine::Get()->Get_Time()->Get_Delta_Seconds();
			if (m_trigger_timer > 0.05f)
			{
				trigger = true;
				m_trigger_timer = 0.0f;
			}
		}

		if (trigger && Tile_Position_In_Boundry(tile_position, pixel_based) && m_camera->Get_Input_Viewport().Intersects(mouse->Get_Position()))
		{
			m_selection_start = tile_position;
			m_selection_end = tile_position;

			result = true;
		}
	}

	Vector2 boundry = m_boundry_size;
	if (pixel_based == true)
	{
		boundry.X *= Get_Tile_Size().X;
		boundry.Y *= Get_Tile_Size().Y;
	}

	if (pixel_based)
	{
		Vector2 start = m_selection_start.MinValue(m_selection_end);
		Vector2 end = m_selection_start.MaxValue(m_selection_end);
		m_selection_rect = Rect2D(start.X, start.Y, (end.X - start.X) + 1, (end.Y - start.Y) + 1);
	}
	else
	{
		Vector2 min_bounds = Vector2(0, 0);
		Vector2 max_bounds = Vector2(boundry.X, boundry.Y);

		Vector2 start = m_selection_start.MinValue(m_selection_end).MinValue(max_bounds).MaxValue(min_bounds);
		Vector2 end = m_selection_start.MaxValue(m_selection_end).MinValue(max_bounds).MaxValue(min_bounds);

		m_selection_rect = Rect2D(start.X, start.Y, (end.X - start.X) + 1, (end.Y - start.Y) + 1);
		m_selection_rect = m_selection_rect.ClampInside(Rect2D(0.0f, 0.0f, boundry.X, boundry.Y));
	}

	m_pixel_based = pixel_based;

	return result;
}

void EditorTileSelection::Draw(const FrameTime& time, RenderPipeline* pipeline)
{
	Vector2 tile_size		= Get_Tile_Size();
	Vector2 cursor_pos		= m_pixel_based == true ? Get_Cursor_Tile_Position(m_pixel_based) : Get_Cursor_Tile_Position(m_pixel_based) * tile_size;
	Rect2D selection_rect	= m_pixel_based == true ? Get_Selection() : Get_Selection() * tile_size;
	Rect2D cursor_rect		=  Rect2D(cursor_pos, tile_size);
	
	float scale = m_camera->Get_Zoom();
	PrimitiveRenderer renderer;

	if (m_selection_in_progress || m_is_selecting)
	{
		renderer.Draw_Solid_Quad(selection_rect, Color(0, 99, 198, 128));
		renderer.Draw_Wireframe_Quad(selection_rect, 0.0f, Color(51, 153, 255, 255), 1.0);
		//renderer.Draw_Wireframe_Quad(selection_rect.Inflate(-(scale * 2), -(scale * 2)), 0.0f, Color::White, 1.0f);
		//renderer.Draw_Wireframe_Quad(selection_rect.Inflate(-(scale * 4), -(scale * 4)), 0.0f, Color::Black, 1.0f);
	}

	if (!m_selection_in_progress && !m_pixel_based)
	{
		renderer.Draw_Solid_Quad(cursor_rect, Color(0, 99, 198, 128));
		renderer.Draw_Wireframe_Quad(cursor_rect, 0.0f, Color(51, 153, 255, 255), 1.0);
		//renderer.Draw_Wireframe_Quad(cursor_rect.Inflate(-(scale * 2), -(scale * 2)), 0.0f, Color::White, 1.0f);
		//renderer.Draw_Wireframe_Quad(cursor_rect.Inflate(-(scale * 4), -(scale * 4)), 0.0f, Color::Black, 1.0f);
	}
}

void EditorTileSelection::Draw_Radius(const FrameTime& time, RenderPipeline* pipeline, float radius)
{
	Vector2 tile_size = Get_Tile_Size();
	Vector2 cursor_pos = m_pixel_based == true ? Get_Cursor_Tile_Position(m_pixel_based) : Get_Cursor_Tile_Position(m_pixel_based) * tile_size;
	Rect2D selection_rect = m_pixel_based == true ? Get_Selection() : Get_Selection() * tile_size;
	Rect2D cursor_rect = Rect2D(cursor_pos, tile_size);

	float scale = m_camera->Get_Zoom();
	PrimitiveRenderer renderer;

	if (m_selection_in_progress || m_is_selecting)
	{
		renderer.Draw_Solid_Oval(cursor_rect.Inflate(radius * 2 * tile_size.X, radius * 2 * tile_size.Y), Color(51, 153, 255, 128));
	}

	if (!m_selection_in_progress && !m_pixel_based)
	{
		renderer.Draw_Solid_Oval(cursor_rect.Inflate(radius * 2 * tile_size.X, radius * 2 * tile_size.Y), Color(0, 99, 198, 128));
	}
}
