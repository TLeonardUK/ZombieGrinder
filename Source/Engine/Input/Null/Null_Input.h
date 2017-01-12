// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_NULLINPUT_
#define _ENGINE_NULLINPUT_

#include "Engine/Input/Input.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Input/Null/Null_KeyboardState.h"
#include "Engine/Input/Null/Null_MouseState.h"
#include "Engine/Input/Null/Null_JoystickState.h"

class Null_Input : public Input
{
	MEMORY_ALLOCATOR(Null_Input, "Input");

private:
	Null_MouseState*	m_mouse_state;
	Null_KeyboardState*	m_keyboard_state;
	Null_JoystickState*	m_joystick_state;
	int					m_source_index;

public:	

	Null_Input(int source_index);

	// Base functions.	
	void Tick(const FrameTime& time);
	
	// Mouse state.
	MouseState* Get_Mouse_State();
	
	// Key state.
	KeyboardState* Get_Keyboard_State();

	// Joystick state.
	std::vector<JoystickState*> Get_Known_Joysticks();
	
};

#endif

