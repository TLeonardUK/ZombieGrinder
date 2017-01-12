// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UICurveEditor.h"
#include "Engine/Renderer/RenderPipeline.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

#include <algorithm>

bool Sort_Curve_Points(const UICurvePoint& a, const UICurvePoint& b)
{
	return (a.x < b.x);
}

UICurveEditor::UICurveEditor()
{
}

UICurveEditor::~UICurveEditor()
{
}

void UICurveEditor::Clear_Points()
{
	m_points.clear();
	m_moving_point = NULL;
}

void UICurveEditor::Add_Point(float x, float y, bool persistent)
{
	UICurvePoint point;
	point.x				= x;
	point.y				= y;
	point.persistent	= persistent;

	m_points.push_back(point);
}

void UICurveEditor::Refresh()
{
	// Calculate screen-space box.
	m_screen_box = Calculate_Screen_Box();

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UICurveEditor::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UICurveEditor::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UICurveEditor::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	PrimitiveRenderer pr;

	MouseState* mouse = Input::Get()->Get_Mouse_State();
	Vector2 mouse_pos = mouse->Get_Position();
	bool mouse_down = mouse->Is_Button_Down(InputBindings::Mouse_Left);
	bool mouse_was_down = mouse->Was_Button_Down(InputBindings::Mouse_Left);
	bool right_mouse_down = mouse->Is_Button_Down(InputBindings::Mouse_Right);

	float padding = 10.0f;

	Rect2D grid_box = Rect2D
	(
		m_screen_box.X + padding,
		m_screen_box.Y + padding,
		m_screen_box.Width - (padding * 2),
		m_screen_box.Height - (padding * 2)
	);

	Vector2 relative_values = Vector2
	(
		Clamp((mouse_pos.X - grid_box.X) / grid_box.Width, 0.0f, 1.0f),
		1.0f - Clamp((mouse_pos.Y - grid_box.Y) / grid_box.Height, 0.0f, 1.0f)
	);

	if (!mouse_down)
	{
		m_moving_point = NULL;
	}

	// Draw background.
	pr.Draw_Solid_Quad(m_screen_box, Color(255, 255, 255, 255));

	// Draw grid lines.
	pr.Draw_Line(Vector3(grid_box.X, grid_box.Y, 0.0f), Vector3(grid_box.X, grid_box.Y + grid_box.Height, 0.0f), 1.0f, Color::Black);
	pr.Draw_Line(Vector3(grid_box.X, grid_box.Y+ grid_box.Height, 0.0f), Vector3(grid_box.X + grid_box.Width, grid_box.Y + grid_box.Height, 0.0f), 1.0f, Color::Black);

	// Draw points.
	UICurvePoint last_point;
	bool resort = false;

	for (std::vector<UICurvePoint>::iterator iter = m_points.begin(); iter != m_points.end(); iter++)
	{
		UICurvePoint point = *iter;

		static const float hot_box_size = 5.0f;

		Rect2D hot_box = Rect2D
		(
			grid_box.X + (grid_box.Width * point.x) - hot_box_size,
			grid_box.Y + (grid_box.Height * (1.0f - point.y)) - hot_box_size,
			hot_box_size * 2,
			hot_box_size * 2
		);
		
		// Moving point?
		if (mouse_down)
		{
			if (m_moving_point != NULL)
			{
				if (m_moving_point->persistent == false)
				{
					m_moving_point->x = relative_values.X;
				}
				m_moving_point->y = relative_values.Y;
				resort = true;
			}
			else if (hot_box.Intersects(mouse_pos))
			{
				m_moving_point = &(*iter);
			}
		}

		// Deleting point?
		if (right_mouse_down && hot_box.Intersects(mouse_pos))
		{
			if (point.persistent == false)
			{
				iter = m_points.erase(iter);				
				resort = true;
				continue;
			}
		}

		// Draw point.
		pr.Draw_Solid_Oval(hot_box, Color::Blue);

		// Draw line connecting us to last one.
		if (iter != m_points.begin())
		{
			pr.Draw_Line(
				Vector3(grid_box.X + (grid_box.Width * last_point.x), grid_box.Y + (grid_box.Height * (1.0f - last_point.y)), 0.0f),
				Vector3(grid_box.X + (grid_box.Width * point.x), grid_box.Y + (grid_box.Height * (1.0f - point.y)), 0.0f),
				1.0f,
				Color::Blue);
		}

		last_point = point;
	}

	// Mouse down but no moving point? Add point.
	if (mouse_down == true && mouse_was_down == false && m_moving_point == NULL && grid_box.Intersects(mouse_pos))
	{
		UICurvePoint point;
		point.x = relative_values.X;
		point.y = relative_values.Y;
		point.persistent = false;
		m_points.push_back(point);
		m_moving_point = &m_points.at(m_points.size() - 1);
		resort = true;
	}

	// Resort points based on time.
	if (resort == true)
	{
		float x = m_moving_point != NULL ? m_moving_point->x : 0;
		float y = m_moving_point != NULL ? m_moving_point->y : 0;

		std::sort(m_points.begin(), m_points.end(), Sort_Curve_Points);	
		
		if (m_moving_point != NULL)
		{
			for (std::vector<UICurvePoint>::iterator iter = m_points.begin(); iter != m_points.end(); iter++)
			{
				UICurvePoint point = *iter;
				if (point.x == x && point.y == y)
				{
					m_moving_point = &(*iter);
					break;
				}
			}
		}

		scene->Dispatch_Event(manager, UIEvent(UIEventType::CurveEditor_Modified, this));
	}

	// Draw children.
 	UIElement::Draw(time, manager, scene);
}
