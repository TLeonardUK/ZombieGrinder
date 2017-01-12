// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_KEYBOARDSTATE_
#define _ENGINE_INPUT_KEYBOARDSTATE_

#include "Engine/Input/InputBindings.h"
#include "Engine/Input/OutputBindings.h"

#include <string>

class KeyboardState
{
	MEMORY_ALLOCATOR(KeyboardState, "Input");

public:
	virtual bool Is_Key_Down	(InputBindings::Type type) = 0;
	virtual bool Was_Key_Down	(InputBindings::Type type) = 0;
	virtual bool Was_Key_Pressed(InputBindings::Type type) = 0;

	virtual bool Is_Action_Down		(OutputBindings::Type type) { return false; }
	virtual bool Was_Action_Down	(OutputBindings::Type type) { return false; }
	virtual bool Was_Action_Pressed	(OutputBindings::Type type) { return false; }

	virtual float Time_Since_Last_Input() = 0;

	virtual std::string Read_Character() = 0;
	virtual void Flush_Character_Stack() = 0;

};

#endif

