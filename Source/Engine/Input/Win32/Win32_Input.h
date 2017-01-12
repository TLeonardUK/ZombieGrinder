// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_WIN32INPUT_
#define _ENGINE_WIN32INPUT_

#include "Engine/Input/Input.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Input/Win32/Win32_KeyboardState.h"
#include "Engine/Input/Win32/Win32_MouseState.h"
#include "Engine/Input/Win32/Win32_JoystickState.h"

class Win32_Input : public Input
{
	MEMORY_ALLOCATOR(Win32_Input, "Input");

private:
	Win32_MouseState*		m_mouse_state;
	Win32_KeyboardState*	m_keyboard_state;
	int						m_source_index;

public:	

	Win32_Input(int source_index);

	// Base functions.	
	void Tick(const FrameTime& time);
	
	// Mouse state.
	MouseState* Get_Mouse_State();
	
	// Key state.
	KeyboardState* Get_Keyboard_State();

	// Joystick state.
	std::vector<JoystickState*> Get_Known_Joysticks();

	void Scan_For_Devices();

};

#endif

#endif