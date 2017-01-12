// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _ENGINE_LINUXINPUT_
#define _ENGINE_LINUXINPUT_

#include "Engine/Input/Input.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Input/Linux/Linux_KeyboardState.h"
#include "Engine/Input/Linux/Linux_MouseState.h"
#include "Engine/Input/Linux/Linux_JoystickState.h"

class Linux_Input : public Input
{
	MEMORY_ALLOCATOR(Linux_Input, "Input");

private:
	Linux_MouseState*		m_mouse_state;
	Linux_KeyboardState*	m_keyboard_state;
	int						m_source_index;

public:	

	Linux_Input(int source_index);

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