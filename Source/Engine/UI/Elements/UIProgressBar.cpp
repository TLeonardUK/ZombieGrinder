// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIProgressBar.h"
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

UIProgressBar::UIProgressBar()
	: m_progress(0)
	, m_color(255, 255, 255, 255)
	, m_fore_color(255, 194, 14, 255)
	, m_bg_color(0, 0, 0, 0)
	, m_draw_progress(0.0f)
	, m_show_progress(true)
	, m_pending_progress(0.0f)
{
}

UIProgressBar::~UIProgressBar()
{
}

void UIProgressBar::Set_Progress(float progress)
{
	m_progress = Min(1.0f, Max(0.0f, progress));
}

void UIProgressBar::Set_Pending_Progress(float progress)
{
	m_pending_progress = Min(1.0f, Max(0.0f, progress));
}

void UIProgressBar::Set_Foreground_Color(Color color)
{
	m_fore_color = color;
}

Color UIProgressBar::Get_Foreground_Color()
{
	return m_fore_color;
}

void UIProgressBar::Set_Color(Color color)
{
	m_color = color;
}

Color UIProgressBar::Get_Color()
{
	return m_color;
}

void UIProgressBar::Set_Background_Color(Color color)
{
	m_bg_color = color;
}

Color UIProgressBar::Get_Background_Color()
{
	return m_bg_color;
}

void UIProgressBar::Refresh()
{
	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	UIManager* manager	= GameEngine::Get()->Get_UIManager();
	m_atlas				= manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, true);
	
	// Grab all atlas frames.
	m_background_frame = UIFrame("screen_main_progress_border_#");
	
	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIProgressBar::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIProgressBar::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Lerp the draw-progress to smoothly scroll the bar.
	m_draw_progress = Math::Lerp(m_draw_progress, m_progress, 0.2f);

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIProgressBar::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);
	float spacing = 2.0f * ui_scale.X;
	float padding = 2.0f * ui_scale.X;

	if (m_progress == 0.0f)
		padding = 0.0f;

	// Draw background frame.
	m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, m_color);

	// Draw progress.
	Rect2D bar_rect = Rect2D(m_screen_box.X + spacing, m_screen_box.Y + spacing, m_screen_box.Width - (spacing * 2), m_screen_box.Height - (spacing * 2));
	m_primitive_renderer.Draw_Solid_Quad(Rect2D(bar_rect.X + (bar_rect.Width * m_draw_progress) + padding, bar_rect.Y, (bar_rect.Width * (1.0f - m_draw_progress)) - padding, bar_rect.Height), m_bg_color * m_color);

	Color error_color = Color::Red;

	if (m_pending_progress > 1.0f)
	{
		m_primitive_renderer.Draw_Solid_Quad(Rect2D(bar_rect.X, bar_rect.Y, (bar_rect.Width * m_draw_progress), bar_rect.Height), error_color * m_color);
	}
	else
	{
		m_primitive_renderer.Draw_Solid_Quad(Rect2D(bar_rect.X, bar_rect.Y, (bar_rect.Width * m_draw_progress), bar_rect.Height), m_fore_color * m_color);
	}

	if (m_pending_progress > m_draw_progress)
	{
		float prog_diff = m_pending_progress - m_draw_progress;

		Color pending_color = Color::Aqua;
		pending_color.A = 128;

		if (m_pending_progress > 1.0f)
		{
			pending_color = error_color;
		}

		m_primitive_renderer.Draw_Solid_Quad(Rect2D(bar_rect.X + (bar_rect.Width * m_draw_progress), bar_rect.Y, (bar_rect.Width * prog_diff), bar_rect.Height), pending_color * m_color);
	}

	// Draw progress text.
	if (m_show_progress == true || m_value != "")
	{
		std::string s = m_value;

		if (s == "")
		{
			char text[64];
			sprintf(text, "%i%%", (int)(m_progress * 100));
			s = text;
		}

		m_font_renderer.Draw_String(s.c_str(), m_screen_box, 16.0f, m_color, TextAlignment::Center, TextAlignment::Center, ui_scale * 0.5f);
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
