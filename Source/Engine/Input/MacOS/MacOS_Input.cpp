// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Input/MacOS/MacOS_Input.h"

MacOS_Input::MacOS_Input(int source_index)
{
	m_source_index	 = source_index;
	m_mouse_state	 = new MacOS_MouseState(source_index);
	m_keyboard_state = new MacOS_KeyboardState(source_index);
}

void MacOS_Input::Tick(const FrameTime& time)
{
	if (m_source_index == 0)
	{
		m_mouse_state->Tick(time);
		m_keyboard_state->Tick(time);
	}

	Scan_For_Devices();

	std::vector<JoystickState*> joysticks = MacOS_JoystickState::Get_All_Joysticks();
	for (std::vector<JoystickState*>::iterator iter = joysticks.begin(); iter != joysticks.end(); iter++)
	{
		MacOS_JoystickState* State = (MacOS_JoystickState*)*iter;
		State->Tick(time);
	}

	Update_Bindings();
}
	
MouseState* MacOS_Input::Get_Mouse_State()
{
	return m_mouse_state;
}
	
KeyboardState* MacOS_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}

std::vector<JoystickState*> MacOS_Input::Get_Known_Joysticks()
{
	return MacOS_JoystickState::Get_Known_Joysticks();
}

void MacOS_Input::Scan_For_Devices()
{
	return MacOS_JoystickState::Scan_For_Devices();
}

#endif