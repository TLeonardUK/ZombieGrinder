// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_NULL_JOYSTICKSTATE_
#define _ENGINE_INPUT_NULL_JOYSTICKSTATE_

#include "Engine/Input/JoystickState.h"
#include "Engine/Engine/FrameTime.h"

class Null_Input;

class Null_JoystickState : public JoystickState
{
	MEMORY_ALLOCATOR(Null_JoystickState, "Input");

private:
	friend class Null_Input;
	
	int m_source_index;
	float m_deadzone;

public:
	Null_JoystickState(int source_index);

	void Set_Vibration(float leftMotor, float rightMotor);

	bool Is_Key_Down(InputBindings::Type type);
	bool Was_Key_Down(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);

	JoystickIconSet::Type Get_Icon_Set();

	std::string Get_UniqueID();

	float Time_Since_Last_Input();

	bool Is_Connected();
	
	std::string Read_Character();
	void Flush_Character_Stack();

};

#endif

