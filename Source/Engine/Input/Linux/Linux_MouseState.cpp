// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/Input/Linux/Linux_MouseState.h"
#include "Engine/Display/Linux/Linux_GfxDisplay.h"

#include <float.h>

float Linux_MouseState::g_wheel_delta = 0.0f;

Linux_MouseState::Linux_MouseState(int source_index)
	: m_source_index(source_index)
	, m_time_since_last_input(FLT_MAX)
{
	memset(m_button_previous_state, 0, sizeof(m_button_previous_state));
	memset(m_button_state, 0, sizeof(m_button_state));
	memset(m_double_clicked, 0, sizeof(m_double_clicked));
}

void Linux_MouseState::Tick(const FrameTime& time)
{
	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	memcpy(m_button_previous_state, m_button_state, sizeof(m_button_state));
	memset(m_double_clicked, 0, sizeof(m_double_clicked));
	memset(m_double_clicked, 0, sizeof(m_double_clicked));

	// If display is not active, we don't want this input.
	Linux_GfxDisplay* display = static_cast<Linux_GfxDisplay*>(GfxDisplay::Get());

	if (display != NULL && !display->Is_Active())
	{
		memset(m_button_state, 0, sizeof(m_button_state));
		memset(m_double_clicked, 0, sizeof(m_double_clicked));
		return;
	}

	// Get mouse position.	
	int mouse_x = 0, mouse_y = 0;
	int abs_mouse_x = 0, abs_mouse_y = 0;
	unsigned int mouse_mask = 0;
	Window root_window, child_window;
	int ret = XQueryPointer(display->m_display, display->m_window, &root_window, &child_window, &abs_mouse_x, &abs_mouse_y, &mouse_x, &mouse_y, &mouse_mask);

	// Get mouse button state.
	m_button_state[0] = (mouse_mask & Button1Mask) != 0;
	m_button_state[1] = (mouse_mask & Button3Mask) != 0; // Fuck knows why right-mouse is 3.
	m_button_state[2] = (mouse_mask & Button2Mask) != 0;
	m_mouse_position = Vector2(mouse_x, mouse_y);

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

bool Linux_MouseState::Is_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Linux_MouseState::Was_Button_Down(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Linux_MouseState::Was_Button_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_button_state[((int)type - InputBindings::MOUSE_START) - 1] && !m_button_previous_state[((int)type - InputBindings::MOUSE_START) - 1];
}

bool Linux_MouseState::Was_Button_Double_Clicked(InputBindings::Type type)
{
	if (m_source_index != 0)
	{
		return false;
	}
	return m_double_clicked[((int)type - InputBindings::MOUSE_START) - 1];
}

void Linux_MouseState::Set_Position(Vector2 position)
{
	if (m_source_index != 0)
	{
		return;
	}

	Linux_GfxDisplay* display = static_cast<Linux_GfxDisplay*>(GfxDisplay::Get());

	XWarpPointer(display->m_display, display->m_window, display->m_window, 0, 0, 0, 0, (int)position.X, (int)position.Y);
}

Vector2 Linux_MouseState::Get_Position()
{
	if (m_source_index != 0)
	{
		return Vector2(0, 0);
	}
	return m_mouse_position;
}

float Linux_MouseState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

float Linux_MouseState::Get_Scroll_Value()
{
	return g_wheel_delta;
}

void Linux_MouseState::Post_Wheel_Event(float delta)
{
	g_wheel_delta = delta;
}

#endif