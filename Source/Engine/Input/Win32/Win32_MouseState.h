// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_INPUT_WIN32_MOUSESTATE_
#define _ENGINE_INPUT_WIN32_MOUSESTATE_

#include "Engine/Input/MouseState.h"
#include "Engine/Engine/FrameTime.h"

class Win32_Input;

class Win32_MouseState : public MouseState
{
	MEMORY_ALLOCATOR(Win32_MouseState, "Input");

private:
	friend class Win32_Input;

	static float g_wheel_delta;

	bool m_button_state[3];
	bool m_button_previous_state[3];
	bool m_double_clicked[3];
	
	static bool m_key_state[256];
		
	float m_time_since_last_input;

	float m_time_since_click[3];
	
	int m_source_index;

	enum
	{
		DOUBLE_CLICK_INTERVAL = 500
	};

public:
	Win32_MouseState(int source_index);

	void	Tick				(const FrameTime& time);

	bool	Is_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Clicked			(InputBindings::Type type);
	bool	Was_Button_Double_Clicked	(InputBindings::Type type);
	
	float	Time_Since_Last_Input();

	void	Set_Position		(Vector2 position);
	Vector2	Get_Position		();

	float	Get_Scroll_Value	();

	static void Post_Wheel_Event(float delta);
	static void Post_Key_Down(int vk);
	static void Post_Key_Up(int vk);

	static void Clear_State();

};

#endif

#endif