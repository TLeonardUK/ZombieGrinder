// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_INPUT_MACOS_MOUSESTATE_
#define _ENGINE_INPUT_MACOS_MOUSESTATE_

#include "Engine/Input/MouseState.h"
#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/Vector2.h"

class MacOS_Input;
class MacOS_GfxDisplay;

class MacOS_MouseState : public MouseState
{
	MEMORY_ALLOCATOR(MacOS_MouseState, "Input");

private:
	friend class MacOS_Input;
	friend class MacOS_GfxDisplay;

	bool m_button_state[3];
	bool m_button_previous_state[3];
	bool m_double_clicked[3];
	
	float m_time_since_last_input;

	float m_time_since_click[3];
	
	int m_source_index;

	Vector2 m_mouse_position;

	enum
	{
		DOUBLE_CLICK_INTERVAL = 500
	};

private:
	static void Post_Wheel_Event(float delta);
	static void Post_Mouse_Up(int button);
	static void Post_Mouse_Down(int button);
	static void Post_Mouse_Position(int x, int y);

public:
	MacOS_MouseState					(int source_index);
	
	void	Tick						(const FrameTime& time);

	bool	Is_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Clicked			(InputBindings::Type type);
	bool	Was_Button_Double_Clicked	(InputBindings::Type type);
	
	float	Time_Since_Last_Input		();

	void	Set_Position				(Vector2 position);
	Vector2	Get_Position				();
	
	float	Get_Scroll_Value			();

};

#endif

#endif