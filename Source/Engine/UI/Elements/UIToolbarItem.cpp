// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIToolbarItem.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIEvent.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

UIToolbarItem::UIToolbarItem()
	: m_state(UIToolbarItemState::Normal)
	, m_icon_name("")
	, m_icon(NULL)
	, m_seperator_height(1)
	, m_seperator(false)
	, m_selected(false)
	, m_icon_color(Color::White)
	, m_was_clicked(false)
{
	m_accepts_mouse_input = true;
}

UIToolbarItem::~UIToolbarItem()
{
}

bool UIToolbarItem::Was_Clicked()
{
	return m_was_clicked;
}

void UIToolbarItem::Refresh()
{
	m_manager			= GameEngine::Get()->Get_UIManager();
	m_atlas				= m_manager->Get_Atlas();
	m_atlas_renderer	= AtlasRenderer(m_atlas);
	m_font				= m_manager->Get_Font();
	m_font_renderer		= FontRenderer(m_font, false, false);
	
	// Grab all atlas frames.
	m_background_hover_frame	 = UIFrame("toolbaritem_background_hover_#");
	m_background_pressed_frame	 = UIFrame("toolbaritem_background_pressed_#");
	m_background_seperator_frame = UIFrame("toolbaritem_background_seperator_#");
	m_background_frame			 = UIFrame("toolbaritem_background_#");
	m_icon					     = m_icon_name == "" ? NULL : m_atlas->Get()->Get_Frame(m_icon_name.c_str());
	
	if (m_icon == NULL)
	{
		m_seperator  = true;
		if (m_value == "")
		{
			m_screen_box = Rect2D(0.0f, 0.0f, 1.0f, (float)m_seperator_height);
		}
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();
}

void UIToolbarItem::After_Refresh()
{		
	// Calculate position of icon.
	if (m_icon != NULL)
	{
		m_icon_position = Vector2(m_screen_box.X + (m_screen_box.Width / 2) - (m_icon->Rect.Width / 2), 
							    m_screen_box.Y + (m_screen_box.Height / 2) - (m_icon->Rect.Height / 2));
	}

	// After-refresh children.
	UIElement::After_Refresh();
}

void UIToolbarItem::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	Input*		input	 = Input::Get();
	MouseState* mouse	 = input->Get_Mouse_State();
	Vector2		position = mouse->Get_Position();

	m_was_clicked = false;

	// Menu item selected?
	if (m_seperator == false && m_enabled == true)
	{
		if (m_screen_box.Intersects(position))
		{
			bool left = mouse->Was_Button_Clicked(InputBindings::Mouse_Left);
			bool right = mouse->Was_Button_Clicked(InputBindings::Mouse_Right);
			if (left == true || right == true)
			{
				if (right == true)
				{
					scene->Dispatch_Event(manager, UIEvent(UIEventType::ToolbarItem_RightClick, this));
				}
				else
				{
					m_was_clicked = true;
					scene->Dispatch_Event(manager, UIEvent(UIEventType::ToolbarItem_Click, this));
				}
			}
			else if (mouse->Is_Button_Down(InputBindings::Mouse_Left) || mouse->Is_Button_Down(InputBindings::Mouse_Right))
			{
				m_state = UIToolbarItemState::Pressed;
			}
			else
			{
				m_state = UIToolbarItemState::Hover;
			}
		}
		else
		{
			m_state = UIToolbarItemState::Normal;
		}
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIToolbarItem::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_seperator == true)
	{
		//m_background_seperator_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
		if (m_value != "")
		{
			FontRenderer fr(manager->Get_Font(), false, false);
			fr.Draw_String(m_value.c_str(), m_screen_box, 16.0f, Color::Black, TextAlignment::Center, TextAlignment::ScaleToFit);
		}
	}
	else
	{
		// Draw background.
		if (m_enabled == false)
		{
			m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
		}
		else if (m_state == UIToolbarItemState::Pressed || m_selected == true)
		{
			m_background_pressed_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
		}
		else if (m_state == UIToolbarItemState::Hover)
		{
			m_background_hover_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
		}
		else
		{
			m_background_frame.Draw_Frame(m_atlas_renderer, m_screen_box);
		}

		// Draw the icon.
		if (m_icon != NULL)
		{
			if (m_enabled == false)
			{
				m_atlas_renderer.Draw_Frame(m_icon, m_icon_position, 0.0f, Color(m_icon_color.R * 0.25f, m_icon_color.G * 0.25f, m_icon_color.B * 0.25f, m_icon_color.A * 0.25f));
			}
			else
			{
				m_atlas_renderer.Draw_Frame(m_icon, m_icon_position, 0.0f, m_icon_color);
			}
		}
	}
}

void UIToolbarItem::Set_Selected(bool value)
{
	m_selected = value;
}

bool UIToolbarItem::Get_Selected()
{
	return m_selected;
}

void UIToolbarItem::Set_Icon_Color(Color color)
{
	m_icon_color = color;
}

void UIToolbarItem::Set_Icon(AtlasFrame* frame)
{
	m_icon = frame;
	m_icon_name = frame->Name;
}