// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_NULL_MOUSESTATE_
#define _ENGINE_INPUT_NULL_MOUSESTATE_

#include "Engine/Input/MouseState.h"
#include "Engine/Engine/FrameTime.h"

class Null_Input;

class Null_MouseState : public MouseState
{
	MEMORY_ALLOCATOR(Null_MouseState, "Input");

private:
	friend class Null_Input;

	int m_source_index;

public:
	Null_MouseState(int source_index);

	bool	Is_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Down				(InputBindings::Type type);
	bool	Was_Button_Clicked			(InputBindings::Type type);
	bool	Was_Button_Double_Clicked	(InputBindings::Type type);
	
	float Time_Since_Last_Input();

	void	Set_Position		(Vector2 position);
	Vector2	Get_Position		();
	
	float	Get_Scroll_Value	();

};

#endif

