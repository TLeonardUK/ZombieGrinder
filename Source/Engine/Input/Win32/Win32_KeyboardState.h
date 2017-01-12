// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_INPUT_WIN32_KEYBOARDSTATE_
#define _ENGINE_INPUT_WIN32_KEYBOARDSTATE_

#include "Engine/Input/KeyboardState.h"
#include "Engine/Engine/FrameTime.h"

#include <vector>

class Win32_Input;
class Win32_GfxDisplay;

class Win32_KeyboardState : public KeyboardState
{
	MEMORY_ALLOCATOR(Win32_KeyboardState, "Input");

private:
	friend class Win32_Input;
	
	bool m_button_state[256];
	bool m_button_previous_state[256];

	static bool m_key_state[256];
		
	float m_time_since_last_input;

	int m_source_index;

	enum
	{
		MAX_CHARACTER_STACK_SIZE = 128 // Blah, if its too small IME input from stuff like japanese will not function correctly.
	};

	static std::vector<std::string> m_character_stack;

protected:
	friend class Win32_GfxDisplay;

	static void Post_Character(std::string value);
	static void Post_Key_Down(int vk);
	static void Post_Key_Up(int vk);

	static void Clear_State();

public:
	Win32_KeyboardState(int source_index);

	void Tick			(const FrameTime& time);
	
	bool Is_Key_Down	(InputBindings::Type type);
	bool Was_Key_Down	(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);
	
	float Time_Since_Last_Input();

	std::string Read_Character();	
	void Flush_Character_Stack();

};

#endif

#endif
