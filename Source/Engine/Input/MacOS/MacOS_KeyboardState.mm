// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Input/MacOS/MacOS_KeyboardState.h"
#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"

#include "Generic/Helper/EndianHelper.h"

#include <float.h>
#include <Carbon/Carbon.h>

// TODO: Fix this code so its keyboard layout agnostic.

std::vector<std::string> MacOS_KeyboardState::m_character_stack;

static const int g_keyboard_key_to_keycode[256] = 
{
	kVK_Escape,

	kVK_CapsLock,
	kVK_ANSI_KeypadClear,
	kVK_F14,
	kVK_F13,
	kVK_F15,
	kVK_Help,
	kVK_Home,
	kVK_Delete,
	kVK_End,

	kVK_PageUp, kVK_PageDown,
		
	kVK_Tab,
	kVK_Shift, kVK_RightShift,
	kVK_Control, kVK_RightControl,
	kVK_Command, kVK_RightControl,
	kVK_Option, kVK_RightOption,
		
	kVK_Space, 
	kVK_Return,
	kVK_ForwardDelete,

	kVK_ANSI_Minus,
	kVK_ANSI_Equal,
	kVK_ANSI_Grave,
	kVK_ANSI_LeftBracket,
	kVK_ANSI_RightBracket,
	kVK_ANSI_Quote,
	kVK_ANSI_Semicolon,
	0,
	kVK_ANSI_Comma,
	kVK_ANSI_Period,
	kVK_ANSI_Slash,
	kVK_ANSI_Backslash,

	kVK_LeftArrow, kVK_RightArrow, kVK_UpArrow, kVK_DownArrow,

	kVK_ANSI_0, kVK_ANSI_1, kVK_ANSI_2, kVK_ANSI_3, kVK_ANSI_4, kVK_ANSI_5, kVK_ANSI_6, kVK_ANSI_7, kVK_ANSI_8, kVK_ANSI_9, 
	kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_F, kVK_ANSI_G, kVK_ANSI_H, kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L, kVK_ANSI_M, kVK_ANSI_N, kVK_ANSI_O, kVK_ANSI_P, kVK_ANSI_Q, kVK_ANSI_R, kVK_ANSI_S, kVK_ANSI_T, kVK_ANSI_U, kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X, kVK_ANSI_Y, kVK_ANSI_Z,

	kVK_F1, kVK_F2, kVK_F3, kVK_F4, kVK_F5, kVK_F6, kVK_F7, kVK_F8, kVK_F9, kVK_F10, kVK_F11, kVK_F12,

	kVK_ANSI_KeypadDivide,
	kVK_ANSI_KeypadMultiply,
	kVK_ANSI_KeypadMinus,
	kVK_ANSI_KeypadPlus,
	kVK_ANSI_Keypad0,
	kVK_ANSI_Keypad1,
	kVK_ANSI_Keypad2,
	kVK_ANSI_Keypad3,
	kVK_ANSI_Keypad4,
	kVK_ANSI_Keypad5,
	kVK_ANSI_Keypad6,
	kVK_ANSI_Keypad7,
	kVK_ANSI_Keypad8,
	kVK_ANSI_Keypad9,
	kVK_ANSI_KeypadDecimal,
	kVK_ANSI_KeypadEnter
};

MacOS_KeyboardState::MacOS_KeyboardState(int source_index)
	: m_source_index(source_index)
	, m_time_since_last_input(FLT_MAX)
{
	memset(m_button_previous_state, 0, sizeof(m_button_previous_state));
	memset(m_button_state, 0, sizeof(m_button_state));
}

void MacOS_KeyboardState::Tick(const FrameTime& time)
{
	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	for (int i = 0; i < 256; i++)
	{
		m_button_previous_state[i] = m_button_state[i];
	}

	// If display is not active, we don't want this input..
	MacOS_GfxDisplay* display = static_cast<MacOS_GfxDisplay*>(GfxDisplay::Get());

	if (display != NULL && !display->Is_Active())
	{
		memset(m_button_state, 0, sizeof(m_button_state));
		return;
	}

    uint32 key_states[4];
    GetKeys((BigEndianUInt32*)&key_states);

	for (int i = 0; i < 128; i++)
	{
		uint32 key_state = key_states[i / 32];
#ifdef PLATFORM_LITTLE_ENDIAN		
//		Swap_Endian<uint32>(&key_state);
#endif
		uint32 state = key_state & (0x1 << (i % 32));

		m_button_state[i] = (state != 0);

		if (m_button_state[i] != m_button_previous_state[i])
		{
			m_time_since_last_input = 0;
		}
	}
}

int MacOS_KeyboardState::Resolve_Key_Index(InputBindings::Type type)
{
	int key_index = (int)(type - InputBindings::KEYBOARD_START) - 1;
	if (key_index < 0 || key_index >= 256)
	{
		return -1;
	}

	int key_code = g_keyboard_key_to_keycode[key_index];
	return key_code;
}

bool MacOS_KeyboardState::Is_Key_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	
	int resolved = Resolve_Key_Index(type);
	if (resolved < 0)
	{
		return false;
	}

	return m_button_state[resolved];
}

bool MacOS_KeyboardState::Was_Key_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	
	int resolved = Resolve_Key_Index(type);
	if (resolved < 0)
	{
		return false;
	}

	return m_button_previous_state[resolved];
}

bool MacOS_KeyboardState::Was_Key_Pressed(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	
	int resolved = Resolve_Key_Index(type);
	if (resolved < 0)
	{
		return false;
	}

	return m_button_state[resolved] && !m_button_previous_state[resolved];
}

void MacOS_KeyboardState::Post_Character(int chr)
{
	// TODO: UNICODE

	char str[2] = { chr, '\0' };
	m_character_stack.push_back(str);
	if (m_character_stack.size() > MAX_CHARACTER_STACK_SIZE)
	{
		m_character_stack.erase(m_character_stack.begin());
	}
}

std::string MacOS_KeyboardState::Read_Character()
{
	if (m_character_stack.size() == 0)
	{
		return "";
	}

	std::string value = m_character_stack.back();
	m_character_stack.pop_back();
	return value;
}

void MacOS_KeyboardState::Flush_Character_Stack()
{
	m_character_stack.clear();
}

float MacOS_KeyboardState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

#endif