// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_UI_UIELEMENT_
#define _ENGINE_UI_UIELEMENT_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Rect2D.h"
#include "Generic/Helper/StringHelper.h"

#include <vector>

class UIManager;
class UIScene;
class UILayout;
class Input;
class JoystickState;

class OnlineUser;

class UIElement
{
	MEMORY_ALLOCATOR(UIElement, "UI");

protected:
	friend class UILayout;
	friend class UIManager;
	
	std::string				m_value;
	int						m_name_hash;
	std::string				m_name;
	Rect2D					m_box;
	Rect2D					m_box_padding;
	Rect2D					m_screen_box;
	std::vector<UIElement*> m_children;

	UIScene*				m_scene;

	bool					m_aspect_scaling;

	UIElement*				m_parent;

	bool					m_accepts_mouse_input;	

	bool					m_enabled;
	bool					m_visible;

	Vector2					m_offset;

	std::string				m_left_element_name;
	UIElement*				m_left_element;
	std::string				m_right_element_name;
	UIElement*				m_right_element;
	std::string				m_up_element_name;
	UIElement*				m_up_element;
	std::string				m_down_element_name;
	UIElement*				m_down_element;

	Vector2					m_scale;

	OnlineUser*				m_select_user;

	Input*					m_select_input;
	JoystickState*			m_select_joystick;

public:
	bool Is_Always_Focused;

	UIElement();
	virtual ~UIElement();
		
	template<typename T>
	T Find_Element(const char* name)
	{
		int hash = StringHelper::Hash(name);
		return Find_Element<T>(hash);
	}
	
	template<typename T>
	T Find_Element(int hash)
	{
		if (m_name_hash == hash)
		{
			return static_cast<T>(this);
		}

		for (std::vector<UIElement*>::iterator iter = m_children.begin(); iter != m_children.end(); iter++)
		{
			UIElement* element = (*iter)->Find_Element<T>(hash);
			if (element != NULL)
			{
				return static_cast<T>(element);
			}
		}	

		return NULL;
	}

	OnlineUser* Selecting_User()
	{
		return m_select_user;
	}

	Input* Selecting_Input()
	{
		return m_select_input;
	}

	JoystickState* Selecting_Joystick()
	{
		return m_select_joystick;
	}
	virtual void Reposition(Rect2D position);

	virtual void Add_Child(UIElement* element);

	virtual void Refresh();
	virtual void After_Refresh();

	virtual int			Get_Name_Hash();
	virtual void		Set_Name(std::string value);
	virtual std::string Get_Name();
	virtual std::string Get_Value();
	virtual void		Set_Value(std::string value);
	virtual Rect2D        Get_Box();
	virtual void		Set_Box(Rect2D box);
	virtual Rect2D        Get_Screen_Box();
	virtual void        Set_Screen_Box(Rect2D value);
	virtual Rect2D		Calculate_Screen_Box(bool ignore_offset = false);
	virtual void		Set_Parent(UIElement* element);
	virtual UIElement*	Get_Parent();

	virtual Vector2		Get_Draw_Scale(UIManager* manager);
	virtual void		Set_Draw_Scale(Vector2 value);
	
	virtual bool		Focus_Up(UIManager* manager);
	virtual bool		Focus_Down(UIManager* manager);
	virtual bool		Focus_Left(UIManager* manager);
	virtual bool		Focus_Right(UIManager* manager);

	virtual Vector2		Get_Focus_Point();
	virtual void		Focus();
	virtual bool		Is_Focusable();

	std::vector<UIElement*>& Get_Elements();

	void Set_Up_Element(UIElement* element);
	void Set_Down_Element(UIElement* element);
	void Set_Left_Element(UIElement* element);
	void Set_Right_Element(UIElement* element);

	bool Is_Focused(UIManager* manager);

	bool Was_Selected();

	void Set_Scene(UIScene* scene);
	UIScene* Get_Scene();

	void Set_Offset(Vector2 offset);
	Vector2 Get_Offset();

	void Set_Enabled(bool value);
	bool Get_Enabled();
	
	void Set_Visible(bool value);
	bool Get_Visible();

	virtual void Tick(const FrameTime& time, UIManager* manager, UIScene* scene);
	virtual void Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	virtual void Post_Draw(const FrameTime& time, UIManager* manager, UIScene* scene);
	
	virtual UIElement* Find_Element_By_Position(UIManager* manager, Vector2 position, bool only_mouse_hot, bool focusable_only);

};

#endif

