// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Null/Null_MouseState.h"
#include "Engine/Display/Null/Null_GfxDisplay.h"

#include <float.h>

Null_MouseState::Null_MouseState(int source_index)
	: m_source_index(source_index)
{
}

bool Null_MouseState::Is_Button_Down(InputBindings::Type type)
{
	return false;
}

bool Null_MouseState::Was_Button_Down(InputBindings::Type type)
{
	return false;
}

bool Null_MouseState::Was_Button_Clicked(InputBindings::Type type)
{
	return false;
}

bool Null_MouseState::Was_Button_Double_Clicked(InputBindings::Type type)
{
	return false;
}

void Null_MouseState::Set_Position(Vector2 position)
{
}

Vector2 Null_MouseState::Get_Position()
{
	return Vector2(0, 0);
}

float Null_MouseState::Time_Since_Last_Input()
{
	return FLT_MAX;
}

float Null_MouseState::Get_Scroll_Value()
{
	return 0;
}