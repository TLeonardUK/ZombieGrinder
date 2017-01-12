// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/Input/Linux/Linux_KeyboardState.h"
#include "Engine/Display/Linux/Linux_GfxDisplay.h"

#include <float.h>

std::vector<std::string> Linux_KeyboardState::m_character_stack;

static const int g_keyboard_key_to_keycode[256] = 
{
	XK_Escape,

	XK_Caps_Lock,
	XK_Num_Lock,
	XK_Scroll_Lock,
	XK_Print,
	XK_Pause,
	XK_Insert,
	XK_Home,
	XK_Delete,
	XK_End,

	XK_Prior, XK_Next,
		
	XK_Tab,
	XK_Shift_L, XK_Shift_R,
	XK_Control_L, XK_Control_R,
	XK_Meta_L, XK_Meta_R,
	XK_Alt_L, XK_Alt_R,
		
	XK_space, 
	XK_Return,
	XK_BackSpace,

	XK_minus,
	XK_plus,
	XK_asciitilde,
	XK_bracketleft,
	XK_bracketright,
	XK_at,
	XK_semicolon,
	0,
	XK_comma,
	XK_period,
	XK_question,
	XK_backslash,

	XK_Left, XK_Right, XK_Up, XK_Down,

	XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9, 
	XK_A, XK_B, XK_C, XK_D, XK_E, XK_F, XK_G, XK_H, XK_I, XK_J, XK_K, XK_L, XK_M, XK_N, XK_O, XK_P, XK_Q, XK_R, XK_S, XK_T, XK_U, XK_V, XK_W, XK_X, XK_Y, XK_Z,

	XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6, XK_F7, XK_F8, XK_F9, XK_F10, XK_F11, XK_F12,

	XK_KP_Divide,
	XK_KP_Multiply,
	XK_KP_Subtract,
	XK_KP_Add,
	XK_KP_0,
	XK_KP_1,
	XK_KP_2,
	XK_KP_3,
	XK_KP_4,
	XK_KP_5,
	XK_KP_6,
	XK_KP_7,
	XK_KP_8,
	XK_KP_9,
	XK_KP_Delete,
	XK_KP_Enter
};

Linux_KeyboardState::Linux_KeyboardState(int source_index)
	: m_source_index(source_index)
	, m_time_since_last_input(FLT_MAX)
{
	memset(m_button_previous_state, 0, sizeof(m_button_previous_state));
	memset(m_button_state, 0, sizeof(m_button_state));
}

void Linux_KeyboardState::Tick(const FrameTime& time)
{
	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	for (int i = 0; i < 256; i++)
	{
		m_button_previous_state[i] = m_button_state[i];
	}

	// If display is not active, we don't want this input..
	Linux_GfxDisplay* display = static_cast<Linux_GfxDisplay*>(GfxDisplay::Get());

	if (display != NULL && !display->Is_Active())
	{
		memset(m_button_state, 0, sizeof(m_button_state));
		return;
	}

	char key_bitfield[32];
	XQueryKeymap(display->m_display, key_bitfield);

	for (int i = 0; i < 256; i++)
	{
		int state = key_bitfield[i / 8] & (0x1 << (i % 8));

		m_button_state[i] = (state != 0);

		if (m_button_state[i] != m_button_previous_state[i])
		{
			m_time_since_last_input = 0;
		}
	}
}

int Linux_KeyboardState::KeySym_To_KeyCode(int keysym)
{
	Linux_GfxDisplay* display = static_cast<Linux_GfxDisplay*>(GfxDisplay::Get());
	int key_code = XKeysymToKeycode(display->m_display, keysym);	// TODO: Maybe slow? Cache?
	DBG_ASSERT(key_code >= 0 && key_code < 256);
	return key_code;
}

int Linux_KeyboardState::Resolve_Key_Index(InputBindings::Type type)
{
	int key_index = (int)(type - InputBindings::KEYBOARD_START) - 1;
	if (key_index < 0 || key_index >= 256)
	{
		return -1;
	}

	int keysym = g_keyboard_key_to_keycode[key_index];
	int key_code = KeySym_To_KeyCode(keysym);
	return key_code;
}

bool Linux_KeyboardState::Is_Key_Down(InputBindings::Type type)
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

bool Linux_KeyboardState::Was_Key_Down(InputBindings::Type type)
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

bool Linux_KeyboardState::Was_Key_Pressed(InputBindings::Type type)
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

void Linux_KeyboardState::Post_Character(std::string value)
{
	m_character_stack.push_back(value);
	if (m_character_stack.size() > MAX_CHARACTER_STACK_SIZE)
	{
		m_character_stack.erase(m_character_stack.begin());
	}
}

std::string Linux_KeyboardState::Read_Character()
{
	if (m_character_stack.size() == 0)
	{
		return "";
	}

	std::string value = m_character_stack.back();
	m_character_stack.pop_back();
	return value;
}

void Linux_KeyboardState::Flush_Character_Stack()
{
	m_character_stack.clear();
}

float Linux_KeyboardState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

#endif