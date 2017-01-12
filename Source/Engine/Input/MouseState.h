// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_MOUSESTATE_
#define _ENGINE_INPUT_MOUSESTATE_

#include "Generic/Types/Vector2.h"

#include "Engine/Input/InputBindings.h"
#include "Engine/Input/OutputBindings.h"

class MouseState
{
	MEMORY_ALLOCATOR(MouseState, "Input");

public:
	virtual bool	Is_Button_Down				(InputBindings::Type type) = 0;
	virtual bool	Was_Button_Down				(InputBindings::Type type) = 0;
	virtual bool	Was_Button_Clicked			(InputBindings::Type type) = 0;
	virtual bool	Was_Button_Double_Clicked	(InputBindings::Type type) = 0;

	virtual bool	Is_Action_Down				(OutputBindings::Type type) { return false; }
	virtual bool	Was_Action_Down				(OutputBindings::Type type) { return false; }
	virtual bool	Was_Action_Pressed			(OutputBindings::Type type) { return false; }

	virtual float Time_Since_Last_Input			() = 0;

	virtual void	Set_Position				(Vector2 position) = 0;
	virtual Vector2	Get_Position				() = 0;

	virtual float	Get_Scroll_Value			() = 0;

};

#endif

