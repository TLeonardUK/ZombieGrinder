// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Input/Win32/Win32_Input.h"

#include "Engine/Profiling/ProfilingManager.h"

Win32_Input::Win32_Input(int source_index)
{
	m_source_index	 = source_index;
	m_mouse_state	 = new Win32_MouseState(source_index);
	m_keyboard_state = new Win32_KeyboardState(source_index);

	if (Win32_JoystickManager::Try_Get() == NULL)
	{
		Win32_JoystickManager::Create();
		Win32_JoystickManager::Get()->Scan_For_Devices();
	}
}

void Win32_Input::Tick(const FrameTime& time)
{
	if (m_source_index == 0)
	{
		{
			PROFILE_SCOPE("Win32 Mouse");
			m_mouse_state->Tick(time);
		}
		{
			
			PROFILE_SCOPE("Win32 Keyboard");
			m_keyboard_state->Tick(time);
		}

		Win32_JoystickManager::Get()->Tick(time);

		std::vector<JoystickState*> joysticks = Get_Known_Joysticks();
		for (std::vector<JoystickState*>::iterator iter = joysticks.begin(); iter != joysticks.end(); iter++)
		{
			Win32_JoystickState* State = (Win32_JoystickState*)*iter;
			State->Tick(time);
		}
	}

	{		
		PROFILE_SCOPE("Update Bindings");
		Update_Bindings();
	}
}
	
MouseState* Win32_Input::Get_Mouse_State()
{
	return m_mouse_state;
}
	
KeyboardState* Win32_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}

void Win32_Input::Scan_For_Devices()
{
	Win32_JoystickManager::Get()->Scan_For_Devices();
}

std::vector<JoystickState*> Win32_Input::Get_Known_Joysticks()
{
	return Win32_JoystickManager::Get()->Get_Known_Joysticks();
}

#endif