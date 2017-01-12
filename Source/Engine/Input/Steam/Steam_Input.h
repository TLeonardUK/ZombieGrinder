// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_STEAMINPUT_
#define _ENGINE_STEAMINPUT_

#include "Engine/Input/Input.h"
#include "Engine/Engine/FrameTime.h"
#include "Engine/Input/Steam/Steam_JoystickState.h"

#include "public/steam/steam_api.h"

class Null_MouseState;
class Null_KeyboardState;
class Steam_JoystickState;

class Steam_Input : public Input
{
	MEMORY_ALLOCATOR(Steam_Input, "Input");

private:
	Null_MouseState* m_mouse_state;
	Null_KeyboardState* m_keyboard_state;

	std::vector<Steam_JoystickState*> m_joysticks;

public:

	Steam_Input();
	~Steam_Input();

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
