// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/UI/UIElement.h"
#include "Engine/UI/UIScene.h"
#include "Engine/UI/UIManager.h"

#include "Engine/Input/Input.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Engine/FrameTime.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Online/OnlinePlatform.h"
#include "Engine/Online/OnlineUser.h"

#include <math.h>

UIElement::UIElement()
	: m_name_hash(0)
	, m_parent(NULL)
	, m_accepts_mouse_input(false)
	, m_enabled(true)
	, m_aspect_scaling(true)
	, m_scene(NULL)
	, m_left_element_name("")
	, m_left_element(NULL)
	, m_right_element_name("")
	, m_right_element(NULL)
	, m_up_element_name("")
	, m_up_element(NULL)
	, m_down_element_name("")
	, m_down_element(NULL)
	, m_scale(1.0f, 1.0f)
	, m_select_user(NULL)
	, m_select_input(NULL)
	, m_select_joystick(NULL)
	, m_visible(true)
	, Is_Always_Focused(false)
{
}

UIElement::~UIElement()
{
	for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		delete *iter;
	}
	m_children.clear();

	m_parent = NULL;
}

std::vector<UIElement*>& UIElement::Get_Elements()
{
	return m_children;
}

Vector2 UIElement::Get_Draw_Scale(UIManager* manager)
{
	return manager->Get_UI_Scale() * m_scale;
}

void UIElement::Set_Draw_Scale(Vector2 value)
{
	m_scale = value;
}

void UIElement::Set_Enabled(bool value)
{
	m_enabled = value;
}

bool UIElement::Get_Enabled()
{
	return m_enabled;
}

void UIElement::Set_Visible(bool value)
{
	m_visible = value;
}

bool UIElement::Get_Visible()
{
	return m_visible;
}

std::string UIElement::Get_Value()
{
	return m_value;
}

void UIElement::Set_Value(std::string value)
{
	m_value = value;
}

int UIElement::Get_Name_Hash()
{
	if (m_name_hash == 0)
	{
		m_name_hash = StringHelper::Hash(m_name.c_str());
	}
	return m_name_hash;
}

void UIElement::Set_Name(std::string value)
{
	m_name = value;
}

std::string UIElement::Get_Name()
{
	return m_name;
}

Rect2D UIElement::Get_Box()
{
	return m_box;
}

void UIElement::Set_Box(Rect2D value)
{
	m_box = value;
}

void UIElement::Set_Parent(UIElement* element)
{
	m_parent = element;
}

UIElement* UIElement::Get_Parent()
{
	return m_parent;
}

void UIElement::Set_Scene(UIScene* scene)
{
	m_scene = scene;
}

UIScene* UIElement::Get_Scene()
{
	return m_scene;
}

bool UIElement::Focus_Up(UIManager* manager)
{
	if (m_up_element != NULL)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		manager->Focus(m_up_element);
		return true;
	}
	return false;
}

bool UIElement::Focus_Down(UIManager* manager)
{
	if (m_down_element != NULL)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		manager->Focus(m_down_element);
		return true;
	}
	return false;
}

bool UIElement::Focus_Left(UIManager* manager)
{
	if (m_left_element != NULL)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		manager->Focus(m_left_element);
		return true;
	}
	return false;
}

bool UIElement::Focus_Right(UIManager* manager)
{
	if (m_right_element != NULL)
	{
		manager->Play_UI_Sound(UISoundType::Click);
		manager->Focus(m_right_element);
		return true;
	}
	return false;
}

void UIElement::Set_Up_Element(UIElement* element)
{
	m_up_element = element;
}

void UIElement::Set_Down_Element(UIElement* element)
{
	m_down_element = element;
}

void UIElement::Set_Left_Element(UIElement* element)
{
	m_left_element = element;
}

void UIElement::Set_Right_Element(UIElement* element)
{
	m_right_element = element;
}

Vector2 UIElement::Get_Focus_Point()
{
	Rect2D box = Get_Screen_Box();
	return Vector2(box.X - (10), box.Y + (box.Height * 0.5f));
}

void UIElement::Focus()
{
	// Overriden by derived classes.
}

bool UIElement::Is_Focusable()
{
	return false;
}

bool UIElement::Is_Focused(UIManager* manager)
{
	return (manager->Get_Focused_Element() == this) || Is_Always_Focused;
}

bool UIElement::Was_Selected()
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	std::vector<OnlineUser*> users = OnlinePlatform::Get()->Get_Users();	
	m_select_user = NULL;
	m_select_input = NULL;
	m_select_joystick = NULL;

	if (users.size() == 0 || 
		users.at(0)->Get_Input_Source().Device_Index < 0)
	{
		// Anyone?
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			Input* input = Input::Get(i);

			// Joystick
			InputBindings::Type joystick_select = input->Output_To_Active_Input(OutputBindings::GUI_Select, InputBindingType::Joystick);
			if (input->Get_Joystick_State()->Was_Key_Pressed(joystick_select) ||
				input->Get_Joystick_State()->Was_Action_Pressed(OutputBindings::GUI_Select))
			{
				m_select_input = input;
				m_select_joystick = input->Get_Joystick_State();
				return true;
			}

			// Anything.
			if (input->Was_Pressed(OutputBindings::GUI_Select))
			{
				m_select_input = input;
				m_select_joystick = NULL;
				return true;
			}

			// Select by mouse. 
			if (input->Get_Mouse_State()->Was_Button_Clicked(InputBindings::Mouse_Left) &&
				Get_Screen_Box().Intersects(input->Get_Mouse_State()->Get_Position()))
			{
				m_select_input = input;
				m_select_joystick = NULL;
				return true;
			}
		}
	}
	else
	{
		// Specific user?
		for (int pass = 0; pass < 2; pass++)
		{
			for (std::vector<OnlineUser*>::iterator iter = users.begin(); iter != users.end(); iter++)
			{
				OnlineUser* user = *iter;		
				InputSource source =  user->Get_Input_Source();

				if (source.Device_Index < 0)
				{
					continue;
				}

				Input* input = Input::Get(source.Device_Index);
		
				// Select by keyboard/joystick.
				if (source.Device == InputSourceDevice::Joystick)
				{
					if (pass == 1)
					{
						continue;
					}

					InputBindings::Type joystick_select = input->Output_To_Active_Input(OutputBindings::GUI_Select, InputBindingType::Joystick);			
					if (input->Get_Joystick_State()->Was_Key_Pressed(joystick_select) ||
						input->Get_Joystick_State()->Was_Action_Pressed(OutputBindings::GUI_Select))
					{
						m_select_user = user; 
						m_select_input = input;
						m_select_joystick = input->Get_Joystick_State();
						return true;
					}
				}
				else
				{
					if (pass == 0)
					{
						continue;
					}

					InputBindings::Type keyboard_select = input->Output_To_Active_Input(OutputBindings::GUI_Select, InputBindingType::Keyboard);			
					if (input->Was_Pressed(OutputBindings::GUI_Select))
					{
						m_select_user = user;
						m_select_input = input;
						m_select_joystick = NULL;
						return true;
					}
				}

				// Select by mouse. 
				if (input->Get_Mouse_State()->Was_Button_Clicked(InputBindings::Mouse_Left) &&
					Get_Screen_Box().Intersects(input->Get_Mouse_State()->Get_Position()))
				{
					m_select_user = user; // Always use first user for mouse selects. (maybe we should always use keyboard bound user!?)
					return true;
				}
			}
		}

		// Anyone?
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			Input* input = Input::Get(i);
			if (input->Was_Pressed(OutputBindings::GUI_Select))
			{
				m_select_user = NULL;
				m_select_input = NULL;
				m_select_joystick = NULL;
				return true;
			}
		}
	}

	return false;
}

void UIElement::Refresh()
{
	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			(*iter)->Refresh();
		}
	}
}

void UIElement::After_Refresh()
{
	if (m_up_element_name != "")
	{
		m_up_element = m_scene->Find_Element<UIElement*>(m_up_element_name.c_str());
	}
	if (m_down_element_name != "")
	{
		m_down_element = m_scene->Find_Element<UIElement*>(m_down_element_name.c_str());
	}
	if (m_left_element_name != "")
	{
		m_left_element = m_scene->Find_Element<UIElement*>(m_left_element_name.c_str());
	}
	if (m_right_element_name != "")
	{
		m_right_element = m_scene->Find_Element<UIElement*>(m_right_element_name.c_str());
	}

	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			(*iter)->After_Refresh();
		}
	}
}

Rect2D UIElement::Get_Screen_Box()
{
	return m_screen_box;
}

void UIElement::Set_Screen_Box(Rect2D value)
{
	m_screen_box = value;
}

Rect2D UIElement::Calculate_Screen_Box(bool ignore_offset)
{
	if (m_parent != NULL)
	{
		Rect2D parent_rect = m_parent->Get_Screen_Box();

		return Rect2D
		(
			(parent_rect.X + (fabs(m_box.X) <= 1.0f ? m_box.X * parent_rect.Width : m_box.X)) + m_box_padding.X,
			(parent_rect.Y + (fabs(m_box.Y) <= 1.0f ? m_box.Y * parent_rect.Height : m_box.Y)) + m_box_padding.Y,
			(fabs(m_box.Width) <= 1.0f ? m_box.Width * parent_rect.Width : m_box.Width) - m_box_padding.X - m_box_padding.Width,
			(fabs(m_box.Height) <= 1.0f ? m_box.Height * parent_rect.Height : m_box.Height) - m_box_padding.Y - m_box_padding.Height
		);
	}
	else
	{
		float screen_width  = (float)GfxDisplay::Get()->Get_Width();
		float screen_height = (float)GfxDisplay::Get()->Get_Height();
		
		float width_43	= screen_height + (screen_height / 3);
		float offset	= (screen_width - width_43) * 0.5f;

		float size_width = screen_width;

		if (m_aspect_scaling == false)
		{
			offset = 0.0f;
		}
		else
		{			
			size_width = width_43;
		}

		Vector2 scale = m_scene->Get_Scale();
		float scale_offset_x = ((screen_width - (screen_width  * scale.X)) * 0.5f);
		float scale_offset_y = ((screen_height - (screen_height * scale.Y)) * 0.5f);

		float offset_x = m_offset.X;
		float offset_y = m_offset.Y;

		if (ignore_offset == true)
		{
			offset_x = 0.0f;
			offset_y = 0.0f;
		}

		return Rect2D
		(
			((fabs(m_box.X) <= 1.0f ? m_box.X * size_width : m_box.X) + offset) + m_box_padding.X - scale_offset_x + offset_x,
			(fabs(m_box.Y) <= 1.0f ? m_box.Y * screen_height : m_box.Y) + m_box_padding.Y - scale_offset_y + offset_y,
			(fabs(m_box.Width) <= 1.0f ? m_box.Width * size_width : m_box.Width) - m_box_padding.X - m_box_padding.Width + (scale_offset_x * 2),
			(fabs(m_box.Height) <= 1.0f ? m_box.Height * screen_height : m_box.Height) - m_box_padding.Y - m_box_padding.Height + (scale_offset_x * 2)
		);
	}
}

void UIElement::Set_Offset(Vector2 offset)
{
	m_offset = offset;
}

Vector2 UIElement::Get_Offset()
{
	return m_offset;
}

void UIElement::Add_Child(UIElement* element)
{
	m_children.push_back(element);
	element->Set_Parent(this);
}

void UIElement::Tick(const FrameTime& time, UIManager* manager, UIScene* scene)
{	
	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			(*iter)->Tick(time, manager, scene);
		}
	}
}

void UIElement::Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIElement* elm = (*iter);
			if (elm->m_visible == true)
				elm->Draw(time, manager, scene);
		}
	}
}

void UIElement::Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene)
{
	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIElement* elm = (*iter);
			if (elm->m_visible == true)
				elm->Post_Draw(time, manager, scene);
		}
	}
}

UIElement* UIElement::Find_Element_By_Position(UIManager* manager, Vector2 position, bool only_mouse_hot, bool focusable_only)
{
	if (m_children.size() > 0)
	{
		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIElement* hit = (*iter)->Find_Element_By_Position(manager, position, only_mouse_hot, focusable_only);
			if (hit != NULL)
			{
				return hit;
			}
		}
	}

	if (m_screen_box.Intersects(position))
	{
		if (m_accepts_mouse_input == true || only_mouse_hot == false)
		{
			if ((manager->Is_Element_Focusable(this) && Is_Focusable()) || focusable_only == false)
			{
				return this;
			}
		}
	}

	return NULL;
}

void UIElement::Reposition(Rect2D position)
{
	m_box = position;
	Refresh();
}