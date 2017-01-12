// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_MACOSINPUT_
#define _ENGINE_MACOSINPUT_

#include "Engine/Input/Input.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Input/MacOS/MacOS_KeyboardState.h"
#include "Engine/Input/MacOS/MacOS_MouseState.h"
#include "Engine/Input/MacOS/MacOS_JoystickState.h"

class MacOS_Input : public Input
{
	MEMORY_ALLOCATOR(MacOS_Input, "Input");

private:
	MacOS_MouseState*		m_mouse_state;
	MacOS_KeyboardState*	m_keyboard_state;
	int						m_source_index;

public:	

	MacOS_Input(int source_index);

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
