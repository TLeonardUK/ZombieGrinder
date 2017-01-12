// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIButton.h"
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
#include "Engine/Online/OnlineUser.h"

#include "Generic/Math/Math.h"

UIButton::UIButton()
	: m_selecting(false)
	, m_frame_color(Color::White)
	, m_hotkey(OutputBindings::COUNT)
	, m_halign(TextAlignment::Center)
	, m_valign(TextAlignment::Center)
	, m_padding(0, 0, 0, 0)
	, m_no_frame(false)
	, m_use_markup(false)
	, m_game_style(true)
	, m_fast_anim(false)
	, m_select_user(NULL)
	, m_select_joystick(NULL)
{
	m_enabled = true;
}

UIButton::~UIButton()
{
}

void UIButton::Refresh()
{
	// Calculate screen-space box.
	m_screen_box		= Calculate_Screen_Box();
	
	UIManager* manager			= GameEngine::Get()->Get_UIManager();
	m_atlas						= manager->Get_Atlas();
	m_atlas_renderer			= AtlasRenderer(m_atlas);
	m_font						= manager->Get_Font();
	m_font_renderer				= FontRenderer(m_font, false, true);
	m_font_renderer_no_shadow	= FontRenderer(m_font, false, false);
	m_markup_font_renderer		= MarkupFontRenderer(m_font);
	
	// Grab all atlas frames.
	m_game_background_frame = UIFrame("screen_main_button_#");
	m_game_background_active_frame = UIFrame("screen_main_button_active_#");
	m_game_background_press_frame = UIFrame("screen_main_button_press_#");
		
	m_background_frame = UIFrame("button_#");
	m_background_active_frame = UIFrame("button_active_#");
	m_background_press_frame = UIFrame("button_press_#");

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIButton::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

bool UIButton::Is_Focusable()
{
	return true;
}

void UIButton::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	// Hotkey button?
	bool force_select = false;
	if (manager->Get_Hot_Keys_Enabled(this))
	{
		if (m_hotkey != OutputBindings::COUNT && manager->Was_Pressed(m_hotkey))
		{
			if (manager->Check_Hotkey_Valid(m_hotkey))
			{
				force_select = true;
			}			
		}
	}

	// Select button?	
	if (Is_Focused(manager) || m_selecting == true || force_select == true)
	{	
		if (m_selecting == false)
		{
			if (Was_Selected() || force_select == true)
			{
				m_select_user = Selecting_User();
				m_select_joystick = Selecting_Joystick();

				if (m_enabled == false)
				{
					manager->Play_UI_Sound(UISoundType::Back);
				}
				else
				{
					m_selecting = true;
					m_select_timer = 0.0f;

					manager->Play_UI_Sound(UISoundType::Select);
					manager->Disable_Navigation();
				}
			}
		}
		else
		{
			m_select_timer += time.Get_Frame_Time();
			if ((m_fast_anim == false && m_select_timer >= SELECT_FLASH_DURATION) ||
				(m_fast_anim == true && m_select_timer >= SELECT_FLASH_FAST_DURATION))
			{
				scene->Dispatch_Event(manager, UIEvent(UIEventType::Button_Click, this, NULL, m_select_user, m_select_joystick));

				m_selecting = false;
				manager->Enable_Navigation();
			}
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIButton::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Vector2 ui_scale = Get_Draw_Scale(manager);

	Color color = m_frame_color;
	Color text_color = Color::White;

	UIFrame back_frame = m_game_background_frame;
	UIFrame press_frame = m_game_background_press_frame;
	UIFrame active_frame = m_game_background_active_frame;
	float font_size = 16.0f;
	Vector2 font_scale = ui_scale * 0.5f;

	if (m_game_style == false)
	{		
		back_frame = m_background_frame;
		press_frame = m_background_press_frame;
		active_frame = m_background_active_frame;
		ui_scale = Vector2(1, 1);
		font_scale = ui_scale;
		font_size = 8.0f;
		text_color = Color::Black;
	}

	m_markup_font_renderer.Set_Shadow(m_game_style);
	m_font_renderer.Set_Shadow(m_game_style);

	if (m_enabled == false)
	{
		color = Color(color.R / 2, color.G / 2, color.B / 2, color.A / 2);
		text_color = Color(128, 128, 128, 128);
	}

	// Draw background frame.
	if (m_no_frame == false)
	{
		if (m_enabled == true && (Is_Focused(manager) || m_selecting == true))
		{	
			if (m_selecting == true)
			{
				int flash = (int)(m_select_timer / SELECT_FLASH_INTERVAL);
				if ((flash % 2) == 0)
				{
					press_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, color);
				}
				else
				{
					back_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, color);
				}
			}
			else
			{
				active_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, color);
			}
		}
		else
		{
			back_frame.Draw_Frame(m_atlas_renderer, m_screen_box, ui_scale, color);
		}
	}

	Rect2D padded_box = Rect2D
	(
		m_screen_box.X + m_padding.X,
		m_screen_box.Y + m_padding.Y,
		m_screen_box.Width + m_padding.Width,
		m_screen_box.Height + m_padding.Height
	);

	// Draw progress text.
	if (m_no_frame == true)
	{
		if (m_enabled == true)
		{
			if (Is_Focused(manager))// || m_selecting == true)
			{
				text_color = Color::White;
			}
			else
			{
				text_color = Color(128, 128, 128, 255);
			}
		}

		m_font_renderer_no_shadow.Draw_String(S(m_value.c_str()), padded_box, font_size, text_color, m_halign, m_valign, font_scale);
	}
	else
	{
		if (m_use_markup == true)
			m_markup_font_renderer.Draw_String(S(m_value.c_str()), padded_box, font_size, text_color, m_halign, m_valign, font_scale);
		else
			m_font_renderer.Draw_String(S(m_value.c_str()), padded_box, font_size, text_color, m_halign, m_valign, font_scale);
	}

	// Draw children.
	UIElement::Draw(time, manager, scene);
}
