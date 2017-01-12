// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Null/Null_KeyboardState.h"
#include "Engine/Display/Null/Null_GfxDisplay.h"

#include <float.h>

Null_KeyboardState::Null_KeyboardState(int source_index)
	: m_source_index(source_index)
{
}

bool Null_KeyboardState::Is_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Null_KeyboardState::Was_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Null_KeyboardState::Was_Key_Pressed(InputBindings::Type type)
{
	return false;
}

std::string Null_KeyboardState::Read_Character()
{
	return "";
}

void Null_KeyboardState::Flush_Character_Stack()
{
}

float Null_KeyboardState::Time_Since_Last_Input()
{
	return FLT_MAX;
}