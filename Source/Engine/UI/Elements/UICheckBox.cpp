// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UICheckBox.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/UI/UIScene.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

#include "Generic/Math/Math.h"

UICheckBox::UICheckBox()
	: m_frame_color(Color::White)
	, m_checked(false)
{
}

UICheckBox::~UICheckBox()
{
}

bool UICheckBox::Get_Checked()
{
	return m_checked;
}

void UICheckBox::Set_Checked(bool checked)
{
	m_checked = checked;
}

void UICheckBox::Refresh()
{
	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	UIManager* manager	= GameEngine::Get()->Get_UIManager();
	m_atlas				= manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, true);
	
	// Grab all atlas frames.
	m_unchecked_frame	= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_radio_button_0");
	m_checked_frame		= ResourceFactory::Get()->Get_Atlas_Frame("screen_main_radio_button_1");
	
	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UICheckBox::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UICheckBox::Is_Focusable()
{
	return true;
}

void UICheckBox::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Select button?	
	if (Is_Focused(manager))
	{	
		if (Was_Selected() && m_enabled)
		{
			m_checked = !m_checked;
			
			scene->Dispatch_Event(manager, UIEvent(UIEventType::CheckBox_Click, this));

			manager->Play_UI_Sound(UISoundType::Select);
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UICheckBox::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);

	Rect2D check_box = Rect2D
	(
		m_screen_box.X,
		m_screen_box.Y,
		m_checked_frame->Rect.Width * ui_scale.X, 
		m_checked_frame->Rect.Height * ui_scale.Y
	);

	check_box.Y += (m_screen_box.Height - check_box.Height) * 0.5f;
	 
	Color border_color = m_frame_color;
	Color text_color = Color::White;
	if (!m_enabled)
	{
		border_color = Color(border_color.R / 2, border_color.G / 2, border_color.B / 2, border_color.A / 2);
		text_color = Color(text_color.R / 2, text_color.G / 2, text_color.B / 2, text_color.A / 2);
	}

	// Draw background frame.
	if (m_checked == true)
	{
		m_atlas_renderer.Draw_Frame(m_checked_frame, check_box, 0.0f, border_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1.0f, 1.0f));
	}
	else
	{
		m_atlas_renderer.Draw_Frame(m_unchecked_frame, check_box, 0.0f, border_color, false, false, RendererOption::E_Src_Alpha_One_Minus_Src_Alpha, Vector2(1.0f, 1.0f));
	}

	// Draw checkbox text.
	m_font_renderer.Draw_String(S(m_value.c_str()), Vector2(check_box.X + check_box.Width + 10, check_box.Y), 16.0f, text_color, Vector2(0,0), ui_scale * 0.5f);

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
