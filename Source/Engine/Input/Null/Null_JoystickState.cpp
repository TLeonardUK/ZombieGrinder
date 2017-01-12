// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Null/Null_JoystickState.h"
#include "Engine/Display/Null/Null_GfxDisplay.h"

#include <float.h>

Null_JoystickState::Null_JoystickState(int source_index)
	: m_source_index(source_index)
	, m_deadzone(0.5f)
{
}

bool Null_JoystickState::Is_Connected()
{
	return true;
}

void Null_JoystickState::Set_Vibration(float leftMotor, float rightMotor)
{

}

bool Null_JoystickState::Is_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Null_JoystickState::Was_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Null_JoystickState::Was_Key_Pressed(InputBindings::Type type)
{
	return false;
}

std::string Null_JoystickState::Read_Character()
{
	return "";
}

void Null_JoystickState::Flush_Character_Stack()
{
}

JoystickIconSet::Type Null_JoystickState::Get_Icon_Set()
{
	return JoystickIconSet::Xbox360;
}

std::string Null_JoystickState::Get_UniqueID()
{
	return "Null_Joystick";
}

float Null_JoystickState::Time_Since_Last_Input()
{
	return FLT_MAX;
}