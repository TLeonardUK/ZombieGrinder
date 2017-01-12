// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _ENGINE_INPUT_LINUX_KEYBOARDSTATE_
#define _ENGINE_INPUT_LINUX_KEYBOARDSTATE_

#include "Engine/Input/KeyboardState.h"
#include "Engine/Engine/FrameTime.h"

#include <vector>

class Linux_Input;
class Linux_GfxDisplay;

class Linux_KeyboardState : public KeyboardState
{
	MEMORY_ALLOCATOR(Linux_KeyboardState, "Input");

private:
	friend class Linux_Input;
	
	bool m_button_state[256];
	bool m_button_previous_state[256];
		
	float m_time_since_last_input;

	int m_source_index;

	enum
	{
		MAX_CHARACTER_STACK_SIZE = 8
	};

	static std::vector<std::string> m_character_stack;

protected:
	friend class Linux_GfxDisplay;

	static void Post_Character(std::string value);

	int KeySym_To_KeyCode(int keysym);
	int Resolve_Key_Index(InputBindings::Type type);

public:
	Linux_KeyboardState(int source_index);
	
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