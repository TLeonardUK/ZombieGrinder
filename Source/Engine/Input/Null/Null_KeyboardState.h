// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_NULL_KEYBOARDSTATE_
#define _ENGINE_INPUT_NULL_KEYBOARDSTATE_

#include "Engine/Input/KeyboardState.h"
#include "Engine/Engine/FrameTime.h"

#include <vector>

class Null_Input;
class Null_Display;

class Null_KeyboardState : public KeyboardState
{
	MEMORY_ALLOCATOR(Null_KeyboardState, "Input");

private:
	friend class Null_Input;
	
	int m_source_index;

protected:
	friend class Null_Display;

public:
	Null_KeyboardState(int source_index);

	bool Is_Key_Down	(InputBindings::Type type);
	bool Was_Key_Down	(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);
	
	float Time_Since_Last_Input();

	std::string Read_Character();	
	void Flush_Character_Stack();

};

#endif

