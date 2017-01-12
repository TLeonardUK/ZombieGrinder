// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_STEAM_JOYSTICKSTATE_
#define _ENGINE_INPUT_STEAM_JOYSTICKSTATE_

#include "Engine/Input/Input.h"
#include "Engine/Input/JoystickState.h"
#include "Engine/Engine/FrameTime.h"

#include "public/steam/steam_api.h"

class Steam_Input;

class Steam_JoystickState : public JoystickState
{
	MEMORY_ALLOCATOR(Steam_JoystickState, "Input");

private:
	friend class Steam_Input;

	ControllerHandle_t m_handle;
	ControllerActionSetHandle_t m_action_set;
	ControllerDigitalActionHandle_t m_actions[OutputBindings::COUNT];

	bool m_connected;

	bool m_has_input;

	bool m_previous_state[OutputBindings::COUNT];
	bool m_current_state[OutputBindings::COUNT];

	float m_time_since_last_input;

	std::string m_mode;
	static std::string g_global_mode;

private:
	bool Has_Input();

	void ChangeMode(std::string Mode);

public:
	Steam_JoystickState(ControllerHandle_t handle);
	~Steam_JoystickState();

	static void Set_Global_Mode(std::string Mode);

	void Tick(const FrameTime& time);

	void Set_Vibration(float leftMotor, float rightMotor);

	bool Is_Key_Down(InputBindings::Type type);
	bool Was_Key_Down(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);

	bool Is_Action_Down(OutputBindings::Type type);
	bool Was_Action_Down(OutputBindings::Type type);
	bool Was_Action_Pressed(OutputBindings::Type type);

	std::string Get_Custom_Action_Icon(OutputBindings::Type type);

	JoystickIconSet::Type Get_Icon_Set();

	std::string Get_UniqueID();

	float Time_Since_Last_Input();

	bool Is_Connected();

	bool Show_Binding_Screen();

	std::string Read_Character();
	void Flush_Character_Stack();

	bool ShowInputDialog(std::string ExistingText, std::string Description, int MaxLength);
	std::string GetInputDialogResult();
	bool IsInputDialogActive();

};

#endif