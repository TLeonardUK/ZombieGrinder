// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UISlider.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

#include "Generic/Math/Math.h"

UISlider::UISlider()
	: m_progress(0)
	, m_draw_progress(0)
	, m_moving_mouse(false)
	, m_direction(UISliderDirection::Horizontal)
	, m_color(112, 154, 209, 255)
	, m_focusable(true)
	, m_game_style(true)
{
}

UISlider::~UISlider()
{
}

void UISlider::Set_Progress(float progress)
{
	m_progress = Min(1.0f,Max(0.0f, progress));
	m_draw_progress = m_progress;
}

void UISlider::Cancel_Movement()
{
	m_moving_mouse = false;
}

float UISlider::Get_Progress()
{
	return m_progress;
}

bool UISlider::Is_Focusable()
{
	return m_focusable;
}

void UISlider::Set_Focusable(bool focusable)
{
	m_focusable = focusable;
}

bool UISlider::Focus_Left(UIManager* manager)
{
	return true;
}

bool UISlider::Focus_Right(UIManager* manager)
{
	return true;
}

void UISlider::Set_Direction(UISliderDirection::Type direction)
{
	m_direction = direction;
}

void UISlider::Refresh()
{
	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	UIManager* manager	= GameEngine::Get()->Get_UIManager();
	m_atlas				= manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, true);
	
	// Grab all atlas frames.
	m_background_frame = UIFrame("screen_main_input_inactive_#");
	m_active_background_frame = UIFrame("screen_main_slider_hover_#");
	m_knob_frame = UIFrame("screen_main_slider_#");
	
	// Get children to calculate their screen box.
	Refresh_Boxes(manager);
	UIElement::Refresh();
}

void UISlider::Refresh_Boxes(UIManager* manager)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);

	float bar_height = 4 * ui_scale.Y;
	float knob_width = 7 * ui_scale.X;

	if (m_direction == UISliderDirection::Horizontal)
	{
		if (m_game_style == true)
		{
			m_bar_box = Rect2D
			(
				m_screen_box.X,
				m_screen_box.Y + (m_screen_box.Height * 0.5f) - (bar_height * 0.5f),
				m_screen_box.Width,
				bar_height
			);

			m_active_bar_box = Rect2D
			(
				m_bar_box.X + (1 * ui_scale.X),
				m_bar_box.Y + (1 * ui_scale.Y),
				(m_bar_box.Width- (2 * ui_scale.X)) * m_draw_progress,
				m_bar_box.Height - (2 * ui_scale.Y)
			);	

			m_knob_box = Rect2D
			(
				(m_active_bar_box.X + m_active_bar_box.Width) - (knob_width * 0.5f),
				m_screen_box.Y,
				knob_width,
				m_screen_box.Height
			);	
		}
		else
		{
			m_bar_box = Rect2D
			(
				m_screen_box.X,
				m_screen_box.Y + (m_screen_box.Height * 0.5f) - (bar_height * 0.5f),
				m_screen_box.Width  - knob_width,
				bar_height
			);

			m_active_bar_box = Rect2D
			(
				m_bar_box.X + (1 * ui_scale.X),
				m_bar_box.Y + (1 * ui_scale.Y),
				(m_bar_box.Width- (2 * ui_scale.X)) * m_draw_progress,
				m_bar_box.Height - (2 * ui_scale.Y)
			);	

			m_knob_box = Rect2D
			(
				(m_active_bar_box.X + m_active_bar_box.Width),
				m_screen_box.Y,
				knob_width,
				m_screen_box.Height
			);
		}
	}
	else
	{
		knob_width *= 2.0f;

		if (m_game_style == true)
		{
			m_bar_box = Rect2D
			(
				m_screen_box.X + (m_screen_box.Width * 0.5f) - (bar_height * 0.5f),
				m_screen_box.Y,
				bar_height,
				m_screen_box.Height
			);

			m_active_bar_box = Rect2D
			(
				m_bar_box.X + (1 * ui_scale.X),
				m_bar_box.Y + (1 * ui_scale.Y),
				m_bar_box.Width - (2 * ui_scale.X),	
				(m_bar_box.Height - (2 * ui_scale.Y)) * m_draw_progress
			);
		
			m_knob_box = Rect2D
			(
				m_screen_box.X,
				(m_active_bar_box.Y + m_active_bar_box.Height) - (knob_width * 0.5f),
				m_screen_box.Width,
				knob_width
			);
		}
		else
		{
			m_bar_box = Rect2D
			(
				m_screen_box.X + (m_screen_box.Width * 0.5f) - (bar_height * 0.5f),
				m_screen_box.Y,
				bar_height,
				m_screen_box.Height - knob_width
			);

			m_active_bar_box = Rect2D
			(
				m_bar_box.X + (1 * ui_scale.X),
				m_bar_box.Y + (1 * ui_scale.Y),
				m_bar_box.Width - (2 * ui_scale.X),	
				(m_bar_box.Height - (2 * ui_scale.Y)) * m_draw_progress
			);
		
			m_knob_box = Rect2D
			(
				m_screen_box.X,
				(m_active_bar_box.Y + m_active_bar_box.Height),
				m_screen_box.Width,
				knob_width
			);
		}
	}
}

void UISlider::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UISlider::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input* input = Input::Get();

	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();

	// Change progress.
	if (Is_Focused(manager))
	{
		float speed = 0.2f * time.Get_Delta_Seconds();

		if (manager->Is_Down(OutputBindings::GUI_Left))
		{
			m_progress = Max(0.0f, m_progress - speed);
		}
		else if (manager->Is_Down(OutputBindings::GUI_Right))
		{
			m_progress = Min(1.0f, m_progress + speed);
		}
	}

	// Is user clicking on knob bar?
	if (input->Is_Down(InputBindings::Mouse_Left) && !manager->Mouse_Selection_Restricted())
	{
		if (m_moving_mouse == false)
		{
			if (m_knob_box.Intersects(mouse_position) &&
				input->Was_Down(InputBindings::Mouse_Left))
			{
				m_mouse_offset = mouse_position - Vector2(m_knob_box.X, m_knob_box.Y);
				m_moving_mouse = true;

				if (m_focusable == true)
				{
					manager->Focus(this);
				}
			}
		}
		else
		{
			Vector2 position = mouse_position - m_mouse_offset;

			if (m_direction == UISliderDirection::Horizontal)
			{
				m_progress = Max(0.0f, Min(1.0f, (position.X - m_bar_box.X) / (m_bar_box.Width)));
			}
			else
			{
				m_progress = Max(0.0f, Min(1.0f, (position.Y - m_bar_box.Y) / (m_bar_box.Height)));
			}
		}
	}
	else
	{
		if (m_moving_mouse == false)
		{
			if (input->Was_Down(InputBindings::Mouse_Left) && m_screen_box.Intersects(mouse_position))
			{
				m_progress = Clamp((mouse_position.Y - m_screen_box.Y) / m_screen_box.Height, 0.0f, 1.0f);
			}
		}
		m_moving_mouse = false;
	}

	// Lerp the draw-progress to smoothly scroll the bar.
	if (m_game_style == true)
		m_draw_progress = Math::Lerp(m_draw_progress, m_progress, 0.5f);
	else
		m_draw_progress = m_progress;

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UISlider::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	
	Input* input = Input::Get();

	MouseState* mouse = input->Get_Mouse_State();
	Vector2 mouse_position = mouse->Get_Position();
	
	// Calculate rendering boxes.
	Refresh_Boxes(manager);

	if (m_game_style == true)
	{
		// Draw background frame.
		m_background_frame.Draw_Frame(m_atlas_renderer, m_bar_box, ui_scale, Color(255, 255, 255, 200));
		m_primitive_renderer.Draw_Solid_Quad(m_active_bar_box, m_color);

		// Draw progress knob.
		if (m_moving_mouse == true || m_knob_box.Intersects(mouse_position) || Is_Focused(manager))
		{
			m_active_background_frame.Draw_Frame(m_atlas_renderer, m_knob_box, ui_scale);
		}
		else
		{
			m_knob_frame.Draw_Frame(m_atlas_renderer, m_knob_box, ui_scale);
		}
	}
	else
	{
		// Draw background frame.
		m_primitive_renderer.Draw_Solid_Quad(m_screen_box, Color(232, 232, 232, 255));

		// Draw progress knob.
		if (m_moving_mouse == true || m_knob_box.Intersects(mouse_position) || Is_Focused(manager))
		{
			m_primitive_renderer.Draw_Solid_Quad(m_knob_box, Color(106, 106, 106, 255));
		}
		else
		{
			m_primitive_renderer.Draw_Solid_Quad(m_knob_box, Color(208, 209, 215, 255));
		}
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
