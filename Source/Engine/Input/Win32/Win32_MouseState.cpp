// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Input/Win32/Win32_MouseState.h"
#include "Engine/Display/Win32/Win32_GfxDisplay.h"

#include <float.h>

float Win32_MouseState::g_wheel_delta = 0.0f;
bool Win32_MouseState::m_key_state[256];		

Win32_MouseState::Win32_MouseState(int source_index)
	: m_source_index(source_index)
	, m_time_since_last_input(FLT_MAX)
{
	memset(m_button_previous_state, 0, sizeof(bool) * 3);
	memset(m_button_state, 0, sizeof(bool) * 3);
	memset(m_key_state, 0, sizeof(bool) * 256);	
	memset(m_double_clicked, 0, sizeof(bool) * 3);
}

void Win32_MouseState::Tick(const FrameTime& time)
{
	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	memcpy(m_button_previous_state, m_button_state, sizeof(bool) * 3);
	memset(m_double_clicked, 0, sizeof(bool) * 3);

	// If display is not active, we don't want this input..
	GfxDisplay* display = GfxDisplay::Get();
	if (display != NULL && !display->Is_Active())
	{
		memset(m_button_state, 0, sizeof(bool) * 3);
		memset(m_key_state, 0, sizeof(bool) * 256);	
		memset(m_double_clicked, 0, sizeof(bool) * 3);
		return;
	}

	m_button_state[0]	= m_key_state[VK_LBUTTON];
	m_button_state[1]	= m_key_state[VK_RBUTTON];
	m_button_state[2]	= m_key_state[VK_MBUTTON];

	for (int i = 0; i < 3; i++)
	{
		bool clicked = Was_Button_Clicked(static_cast<InputBindings::Type>(InputBindings::MOUSE_START + 1 + i));

		if (clicked == true)
		{
			if (m_time_since_click[i] < DOUBLE_CLICK_INTERVAL)
			{
				m_double_clicked[i] = true;
			}
			m_time_since_click[i] = 0.0f;
		}
		else
		{
			m_time_since_click[i] += time.Get_Frame_Time();
		}

		if (m_button_state[i] == true)
		{
			m_time_since_last_input = 0;
		}
	}

	g_wheel_delta = 0.0f;
}

bool Win32_MouseState::Is_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Win32_MouseState::Was_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Win32_MouseState::Was_Button_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1] && !m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Win32_MouseState::Was_Button_Double_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_double_clicked[((int)type - InputBindings::MOUSE_START) - 1];
}

void Win32_MouseState::Set_Position(Vector2 position)
{
	if (m_source_index != 0)
	{
		return;
	}

	Win32_GfxDisplay* display = static_cast<Win32_GfxDisplay*>(GfxDisplay::Get());
	HWND window_handle = display->Get_Window_Handle();

	POINT point;
	point.x = (int)position.X;
	point.y = (int)position.Y;

	ClientToScreen(window_handle, &point);
	SetCursorPos(point.x, point.y);
}

Vector2 Win32_MouseState::Get_Position()
{
	static Vector2 last_position = Vector2(0, 0);

	if (m_source_index != 0)
	{
		return Vector2(0, 0);
	}

	Win32_GfxDisplay* display = static_cast<Win32_GfxDisplay*>(GfxDisplay::Get());
	HWND window_handle = display->Get_Window_Handle();

	if (display != NULL && !display->Is_Active())
	{
		return last_position;
	}

	POINT point;
	GetCursorPos(&point);
	ScreenToClient(window_handle, &point);

	last_position = Vector2((float)point.x, (float)point.y);

	return last_position;
}

float Win32_MouseState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

float Win32_MouseState::Get_Scroll_Value()
{
	return g_wheel_delta;
}

 void Win32_MouseState::Post_Wheel_Event(float delta)
 {
	 g_wheel_delta = delta;
 }

void Win32_MouseState::Post_Key_Down(int vk)
{
	m_key_state[vk] = true;
}

void Win32_MouseState::Post_Key_Up(int vk)
{
	m_key_state[vk] = false;
}

void Win32_MouseState::Clear_State()
{
	for (int i = 0; i < 256; i++)
	{
		m_key_state[i] = false;
	}
}


#endif