// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_INPUT_MACOS_KEYBOARDSTATE_
#define _ENGINE_INPUT_MACOS_KEYBOARDSTATE_

#include "Engine/Input/KeyboardState.h"
#include "Engine/Engine/FrameTime.h"

#include <vector>

class MacOS_Input;
class MacOS_Display;

class MacOS_KeyboardState : public KeyboardState
{
	MEMORY_ALLOCATOR(MacOS_KeyboardState, "Input");

private:
	friend class MacOS_Input;
	
	int m_source_index;
	
	char m_button_state[256];
	char m_button_previous_state[256]; 

	float m_time_since_last_input;
	
	enum
	{
		MAX_CHARACTER_STACK_SIZE = 8
	};

	static std::vector<std::string> m_character_stack;

protected:
	friend class MacOS_GfxDisplay;

	static void Post_Character(int chr);
	
	int Resolve_Key_Index(InputBindings::Type type);

public:
	MacOS_KeyboardState(int source_index);
	
	void Tick(const FrameTime& time);

	bool Is_Key_Down	(InputBindings::Type type);
	bool Was_Key_Down	(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);
	
	float Time_Since_Last_Input();

	std::string Read_Character();	
	void Flush_Character_Stack();

};

#endif

#endif
