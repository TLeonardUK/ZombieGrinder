// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/Input/Linux/Linux_Input.h"

Linux_Input::Linux_Input(int source_index)
{
	m_source_index	 = source_index;
	m_mouse_state	 = new Linux_MouseState(source_index);
	m_keyboard_state = new Linux_KeyboardState(source_index);
}

void Linux_Input::Tick(const FrameTime& time)
{
	if (m_source_index == 0)
	{
		m_mouse_state->Tick(time);
		m_keyboard_state->Tick(time);
	}

	Scan_For_Devices();

	std::vector<JoystickState*> joysticks = Linux_JoystickState::Get_All_Joysticks();
	for (std::vector<JoystickState*>::iterator iter = joysticks.begin(); iter != joysticks.end(); iter++)
	{
		Linux_JoystickState* State = (Linux_JoystickState*)*iter;
		State->Tick(time);
	}

	Update_Bindings();
}
	
MouseState* Linux_Input::Get_Mouse_State()
{
	return m_mouse_state;
}
	
KeyboardState* Linux_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}

std::vector<JoystickState*> Linux_Input::Get_Known_Joysticks()
{
	return Linux_JoystickState::Get_Known_Joysticks();
}

void Linux_Input::Scan_For_Devices()
{
	return Linux_JoystickState::Scan_For_Devices();
}
	
#endif