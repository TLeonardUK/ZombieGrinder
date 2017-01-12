// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/Elements/UIDropDownMenu.h"
#include "Engine/Renderer/Atlases/Atlas.h"
#include "Engine/Renderer/Atlases/AtlasHandle.h"
#include "Engine/Renderer/Atlases/AtlasRenderer.h"
#include "Engine/Renderer/Textures/TextureHandle.h"
#include "Engine/Renderer/Textures/Texture.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/UI/UIFrame.h"
#include "Engine/UI/UIManager.h"
#include "Engine/Engine/GameEngine.h"
#include "Engine/Input/Input.h"
#include "Engine/Platform/Platform.h"
#include "Engine/Localise/Locale.h"

UIDropDownMenu::UIDropDownMenu()
	: m_frame_name("screen_main_box_inactive_#")
	, m_frame_color(Color::White)
	, m_visible(false)
	, m_previous_focused_element(NULL)
	, m_visible_last_frame(false)
{
	m_aspect_scaling = false;
}

UIDropDownMenu::~UIDropDownMenu()
{
}

void UIDropDownMenu::Refresh()
{
	// Calculate screen-space box.
	m_screen_box = Calculate_Screen_Box();

	// Load background image.
	if (m_frame_name != "")
	{
		m_frame = UIFrame(m_frame_name);
	}

	// Get children to calculate their screen box.
	UIElement::Refresh();

	// Bind all elements so you don't go off the drop-down menu.
	if (m_children.size() > 0)
	{
		for (int i = 0; i < (int)m_children.size(); i++)
		{
			UIElement* prev_element = m_children.at(i <= 0 ? (int)m_children.size() - 1 : i - 1);
			UIElement* element		= m_children.at(i);
			UIElement* next_element = m_children.at(i >= (int)m_children.size() - 1 ? 0 : i + 1);

			element->Set_Down_Element(next_element);
			element->Set_Up_Element(prev_element);
			element->Set_Left_Element(prev_element);
			element->Set_Right_Element(next_element);
		}
	}
}

void UIDropDownMenu::After_Refresh()
{
	// After-refresh children.
	UIElement::After_Refresh();
}

void UIDropDownMenu::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_visible == false || m_visible_last_frame == false)
	{
		m_visible_last_frame = m_visible;
		return;
	}

	if (Input::Get()->Get_Mouse_State()->Was_Button_Clicked(InputBindings::Mouse_Left) &&
		!m_screen_box.Intersects(Input::Get()->Get_Mouse_State()->Get_Position()))
	{
		Hide();
		manager->Play_UI_Sound(UISoundType::Close_Popup);
	}

	// Update children.
	UIElement::Tick(time, manager, scene);
}

void UIDropDownMenu::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_visible == false || m_visible_last_frame == false)
	{
		m_visible_last_frame = m_visible;
		return;
	}

	// Draw background frame.
	m_frame.Draw_Frame_Flat_Top(m_atlas_renderer, m_screen_box, manager->Get_UI_Scale(), m_frame_color);

	// Draw children.
	UIElement::Draw(time, manager, scene);
}

void UIDropDownMenu::Show(Rect2D position)
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	manager->Limit_Mouse_Selection_To_Element(this);

	m_visible_last_frame = false;
	m_visible = true;
	m_box = position;

	// Focus on first element.
	m_previous_focused_element = manager->Get_Focused_Element();
	if (m_children.size() > 0)
	{
		manager->Focus(m_children.at(0));
	}
	
	Refresh();
}

void UIDropDownMenu::Hide()
{
	UIManager* manager = GameEngine::Get()->Get_UIManager();

	manager->Limit_Mouse_Selection_To_Element(NULL);	// TODO: Store and retrieve, so we can have nested drop down menus.

	m_visible = false;

	manager->Focus(m_previous_focused_element);
}

