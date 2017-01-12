// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Input/MacOS/MacOS_MouseState.h"
#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"

#include <float.h>

float g_wheel_delta = 0.0f;
bool  g_mouse_state[3] = { false, false, false };
Vector2 g_mouse_position = Vector2(0, 0);

MacOS_MouseState::MacOS_MouseState(int source_index)
	: m_source_index(source_index)
	, m_time_since_last_input(FLT_MAX)
{
	memset(m_button_previous_state, 0, sizeof(m_button_previous_state));
	memset(m_button_state, 0, sizeof(m_button_state));
	memset(m_double_clicked, 0, sizeof(m_double_clicked));
}

void MacOS_MouseState::Tick(const FrameTime& time)
{
	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	memcpy(m_button_previous_state, m_button_state, sizeof(m_button_state));
	memset(m_double_clicked, 0, sizeof(m_double_clicked));

	// If display is not active, we don't want this input.
	MacOS_GfxDisplay* display = static_cast<MacOS_GfxDisplay*>(GfxDisplay::Get());

	if (display != NULL && !display->Is_Active())
	{
		memset(m_button_state, 0, sizeof(m_button_state));
		memset(m_double_clicked, 0, sizeof(m_double_clicked));
		return;
	}

	// Get mouse position.	
	m_mouse_position = g_mouse_position;

	// Get mouse button state.
	m_button_state[0] = g_mouse_state[0];
	m_button_state[1] = g_mouse_state[1]; 
	m_button_state[2] = g_mouse_state[2];

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

bool MacOS_MouseState::Is_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool MacOS_MouseState::Was_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool MacOS_MouseState::Was_Button_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1] && !m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool MacOS_MouseState::Was_Button_Double_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_double_clicked[((int)type - InputBindings::MOUSE_START) - 1];
}

void MacOS_MouseState::Set_Position(Vector2 position)
{
	if (m_source_index != 0)
	{
		return;
	}

	MacOS_GfxDisplay* display = static_cast<MacOS_GfxDisplay*>(GfxDisplay::Get());

	// TODO
}

Vector2 MacOS_MouseState::Get_Position()
{
	if (m_source_index != 0)
	{
		return Vector2(0, 0);
	}
	return m_mouse_position;
}

float MacOS_MouseState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

float MacOS_MouseState::Get_Scroll_Value()
{
	return g_wheel_delta;
}

void MacOS_MouseState::Post_Wheel_Event(float delta)
{
	g_wheel_delta = delta;
}

void MacOS_MouseState::Post_Mouse_Up(int button)
{
	if (button < 0 || button >= 3)
		return;
	g_mouse_state[button] = false;
}

void MacOS_MouseState::Post_Mouse_Down(int button)
{
	if (button < 0 || button >= 3)
		return;
	g_mouse_state[button] = true;
}

void MacOS_MouseState::Post_Mouse_Position(int x, int y)
{
	g_mouse_position.X = x;
	g_mouse_position.Y = y;
}


#endif