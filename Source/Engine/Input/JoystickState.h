// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_JOYSTICKSTATE_
#define _ENGINE_INPUT_JOYSTICKSTATE_

#include "Engine/Input/InputBindings.h"
#include "Engine/Input/OutputBindings.h"

#include <string>

class JoystickState
{
	MEMORY_ALLOCATOR(JoystickState, "Input");

private:
	friend class Input;

	int m_preferred_index;

public:
	JoystickState()
		: m_preferred_index(-1)
	{
	}

	virtual bool Is_Key_Down	(InputBindings::Type type) = 0;
	virtual bool Was_Key_Down	(InputBindings::Type type) = 0;
	virtual bool Was_Key_Pressed(InputBindings::Type type) = 0;

	virtual bool Is_Action_Down		(OutputBindings::Type type) { return false; }
	virtual bool Was_Action_Down	(OutputBindings::Type type) { return false; }
	virtual bool Was_Action_Pressed	(OutputBindings::Type type) { return false; }

	virtual void Set_Vibration	(float leftMotor, float rightMotor) = 0;

	virtual std::string Get_Custom_Action_Icon(OutputBindings::Type type) { return ""; }

	virtual bool Is_Connected	() = 0;
	
	virtual float Time_Since_Last_Input() = 0;

	virtual std::string Read_Character() = 0;
	virtual void Flush_Character_Stack() = 0;

	virtual std::string Get_UniqueID() = 0;

	virtual JoystickIconSet::Type Get_Icon_Set() = 0;

	virtual bool ShowInputDialog(std::string ExistingText, std::string Description, int MaxLength) { return false; }
	virtual std::string GetInputDialogResult() { return ""; }
	virtual bool IsInputDialogActive() { return false; }

};

#endif

