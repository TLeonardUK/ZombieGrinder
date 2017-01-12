// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Null/Null_Input.h"

Null_Input::Null_Input(int source_index)
{
	m_source_index	 = source_index;
	m_mouse_state	 = new Null_MouseState(source_index);
	m_keyboard_state = new Null_KeyboardState(source_index);
}

void Null_Input::Tick(const FrameTime& time)
{
	Update_Bindings();
}
	
MouseState* Null_Input::Get_Mouse_State()
{
	return m_mouse_state;
}
	
KeyboardState* Null_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}

std::vector<JoystickState*> Null_Input::Get_Known_Joysticks()
{
	std::vector<JoystickState*> states;
	return states;
}