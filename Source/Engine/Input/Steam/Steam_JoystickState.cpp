// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Steam/Steam_JoystickState.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Engine/Online/OnlinePlatform.h"

std::string Steam_JoystickState::g_global_mode = "InMenu";

Steam_JoystickState::Steam_JoystickState(ControllerHandle_t handle)
	: m_handle(handle) 
	, m_time_since_last_input(0.0f)
	, m_has_input(false)
	, m_connected(false)
	, m_mode("")
{
	memset(m_current_state, 0, sizeof(bool) * OutputBindings::COUNT);
	memset(m_previous_state, 0, sizeof(bool) * OutputBindings::COUNT);

	memset(m_actions, 0, sizeof(ControllerActionSetHandle_t) * OutputBindings::COUNT);

	ChangeMode("InMenu");

	/*
	if (m_in_menu_action_set)
	{
		//SteamController()->ActivateActionSet(m_handle, m_in_menu_action_set);

		for (int i = 0; i < OutputBindings::COUNT; i++)
		{
			if (!(i >= OutputBindings::GUI_START && i <= OutputBindings::GUI_END))
			{
				m_menu_actions[i] = NULL;
				continue;
			}

			const char* Name = "";
			OutputBindings::Type Type = (OutputBindings::Type)i;
			OutputBindings::ToString(Type, Name);
			m_menu_actions[i] = SteamController()->GetDigitalActionHandle(Name);

			if (m_menu_actions[i])
			{
				DBG_LOG("Menu Digital Action '%s' is mapped to steam controller.", Name);
			}
		}
	}*/
}

Steam_JoystickState::~Steam_JoystickState()
{
}

void Steam_JoystickState::Set_Global_Mode(std::string Mode)
{
	if (g_global_mode != Mode)
	{
		DBG_LOG("[Joystick] Changed steam action set to '%s'", Mode.c_str());
	}
	g_global_mode = Mode;
}

void Steam_JoystickState::ChangeMode(std::string Mode)
{
	m_action_set = SteamController()->GetActionSetHandle(Mode.c_str());

	DBG_LOG("[Steam Joystick] Changing action set to %s (exists=%i).", Mode.c_str(), m_action_set != NULL)

	for (int i = 0; i < OutputBindings::COUNT; i++)
	{
		if (Mode == "InGame")
		{
			if (i >= OutputBindings::GUI_START && i <= OutputBindings::GUI_END)
			{
				m_actions[i] = NULL;
				continue;
			}
		}
		else
		{
			if (!(i >= OutputBindings::GUI_START && i <= OutputBindings::GUI_END))
			{
				m_actions[i] = NULL;
				continue;
			}
		}

		const char* Name = "";
		OutputBindings::Type Type = (OutputBindings::Type)i;
		OutputBindings::ToString(Type, Name);

		m_actions[i] = SteamController()->GetDigitalActionHandle(Name);

		if (m_actions[i])
		{
			DBG_LOG("Game digital action '%s' is mapped to steam controller.", Name);
		}
	}

	SteamController()->ActivateActionSet(m_handle, m_action_set);

	m_mode = Mode;
}

bool Steam_JoystickState::Is_Connected()
{
	return m_connected;
}

bool Steam_JoystickState::Has_Input()
{
	return m_has_input;
}

void Steam_JoystickState::Tick(const FrameTime& time)
{
	m_has_input = false;

	if (g_global_mode != m_mode)
	{
		ChangeMode(g_global_mode);
	}

	for (int i = 0; i < OutputBindings::COUNT; i++)
	{
		m_previous_state[i] = m_current_state[i];
	}

	for (int i = 0; i < OutputBindings::COUNT; i++)
	{
		OutputBindings::Type Type = (OutputBindings::Type)i;

		ControllerDigitalActionData_t Data;
		Data.bState = false;

		Data = SteamController()->GetDigitalActionData(m_handle, m_actions[i]);

		if (Data.bActive)
		{
			m_current_state[i] = Data.bState;
			
			if (Data.bState)
			{
				EControllerActionOrigin Origins[STEAM_CONTROLLER_MAX_ORIGINS];
				int Count = SteamController()->GetDigitalActionOrigins(m_handle, m_action_set, m_actions[i], Origins);
				/*
				std::string msg = "";
				for (int i = 0; i < Count; i++)
				{
					if (i != 0)
					{
						msg += ", ";
					}

					switch (Origins[i])
					{
						case k_EControllerActionOrigin_None: msg += "None"; break;
						case k_EControllerActionOrigin_A: msg += "A"; break;
						case k_EControllerActionOrigin_B: msg += "B"; break;
						case k_EControllerActionOrigin_X: msg += "X"; break;
						case k_EControllerActionOrigin_Y: msg += "Y"; break;
						case k_EControllerActionOrigin_LeftBumper: msg += "LeftBumper"; break;
						case k_EControllerActionOrigin_RightBumper: msg += "RightBumper"; break;
						case k_EControllerActionOrigin_LeftGrip: msg += "LeftGrip"; break;
						case k_EControllerActionOrigin_RightGrip: msg += "RightGrip"; break;
						case k_EControllerActionOrigin_Start: msg += "Start"; break;
						case k_EControllerActionOrigin_Back: msg += "Back"; break;
						case k_EControllerActionOrigin_LeftPad_Touch: msg += "LeftPad_Touch"; break;
						case k_EControllerActionOrigin_LeftPad_Swipe: msg += "LeftPad_Swipe"; break;
						case k_EControllerActionOrigin_LeftPad_Click: msg += "LeftPad_Click"; break;
						case k_EControllerActionOrigin_LeftPad_DPadNorth: msg += "LeftPad_DPadNorth"; break;
						case k_EControllerActionOrigin_LeftPad_DPadSouth: msg += "LeftPad_DPadSouth"; break;
						case k_EControllerActionOrigin_LeftPad_DPadWest: msg += "LeftPad_DPadWest"; break;
						case k_EControllerActionOrigin_LeftPad_DPadEast: msg += "LeftPad_DPadEast"; break;
						case k_EControllerActionOrigin_RightPad_Touch: msg += "RightPad_Touch"; break;
						case k_EControllerActionOrigin_RightPad_Swipe: msg += "RightPad_Swipe"; break;
						case k_EControllerActionOrigin_RightPad_Click: msg += "RightPad_Click"; break;
						case k_EControllerActionOrigin_RightPad_DPadNorth: msg += "RightPad_DPadNorth"; break;
						case k_EControllerActionOrigin_RightPad_DPadSouth: msg += "RightPad_DPadSouth"; break;
						case k_EControllerActionOrigin_RightPad_DPadWest: msg += "RightPad_DPadWest"; break;
						case k_EControllerActionOrigin_RightPad_DPadEast: msg += "RightPad_DPadEast"; break;
						case k_EControllerActionOrigin_LeftTrigger_Pull: msg += "LeftTrigger_Pull"; break;
						case k_EControllerActionOrigin_LeftTrigger_Click: msg += "LeftTrigger_Click"; break;
						case k_EControllerActionOrigin_RightTrigger_Pull: msg += "RightTrigger_Pull"; break;
						case k_EControllerActionOrigin_RightTrigger_Click: msg += "RightTrigger_Click"; break;
						case k_EControllerActionOrigin_LeftStick_Move: msg += "LeftStick_Move"; break;
						case k_EControllerActionOrigin_LeftStick_Click: msg += "LeftStick_Click"; break;
						case k_EControllerActionOrigin_LeftStick_DPadNorth: msg += "LeftStick_DPadNorth"; break;
						case k_EControllerActionOrigin_LeftStick_DPadSouth: msg += "LeftStick_DPadSouth"; break;
						case k_EControllerActionOrigin_LeftStick_DPadWest: msg += "LeftStick_DPadWest"; break;
						case k_EControllerActionOrigin_LeftStick_DPadEast: msg += "LeftStick_DPadEast"; break;
						case k_EControllerActionOrigin_Gyro_Move: msg += "Gyro_Move"; break;
						case k_EControllerActionOrigin_Gyro_Pitch: msg += "Gyro_Pitch"; break;
						case k_EControllerActionOrigin_Gyro_Yaw: msg += "Gyro_Yaw"; break;
						case k_EControllerActionOrigin_Gyro_Roll: msg += "Gyro_Roll"; break;
					}
				}

				const char* res = "";
				OutputBindings::ToString(Type, res);
				DBG_LOG("Action=%s Origin=%s", res, msg.c_str());
				*/
			}
		}

		if (m_current_state[i] != m_previous_state[i])
		{
			m_has_input = true;
		}
	}

	if (m_has_input)
	{
		m_time_since_last_input = 0.0f;
	}
	else
	{
		m_time_since_last_input += time.Get_Delta_Seconds();
	}
}

void Steam_JoystickState::Set_Vibration(float leftMotor, float rightMotor)
{
	// Nah, this isn't right.
	/*
	if (leftMotor > 0.0f)
	{
		SteamController()->TriggerHapticPulse(m_handle, k_ESteamControllerPad_Left, (unsigned short)(290 * 1000));
	}
	if (rightMotor > 0.0f)
	{
		SteamController()->TriggerHapticPulse(m_handle, k_ESteamControllerPad_Right, (unsigned short)(290 * 1000));
	}
	*/
}

std::string Steam_JoystickState::Get_Custom_Action_Icon(OutputBindings::Type type)
{
	EControllerActionOrigin origins[STEAM_CONTROLLER_MAX_ORIGINS];
	int count = 0;

	if (m_actions[(int)type] != NULL)
	{
		count = SteamController()->GetDigitalActionOrigins(m_handle, m_action_set, m_actions[(int)type], origins);
	}

	if (count > 0)
	{
		static const char* origin_atlas[k_EControllerActionOrigin_Count] = {
			"None",
			"joystick_a_Steam",
			"joystick_b_Steam",
			"joystick_x_Steam",
			"joystick_y_Steam",
			"joystick_lb_Steam",
			"joystick_rb_Steam",
			"joystick_lg_Steam",
			"joystick_rg_Steam",
			"joystick_start_Steam",
			"joystick_back_Steam",
			"joystick_leftpad_touch_Steam",
			"joystick_leftpad_swipe_Steam",
			"joystick_leftpad_click_Steam",
			"joystick_leftpad_swipe_dpadnorth_Steam",
			"joystick_leftpad_swipe_dpadsouth_Steam"
			"joystick_leftpad_swipe_dpadwest_Steam",
			"joystick_leftpad_swipe_dpadeast_Steam",
			"joystick_rightpad_touch_Steam",
			"joystick_rightpad_swipe_Steam",
			"joystick_rightpad_click_Steam",
			"joystick_rightpad_swipe_dpadnorth_Steam",
			"joystick_rightpad_swipe_dpadsouth_Steam",
			"joystick_rightpad_swipe_dpadwest_Steam",
			"joystick_rightpad_swipe_dpadeast_Steam",
			"joystick_lefttrigger_pull_Steam",
			"joystick_lefttrigger_click_Steam",
			"joystick_righttrigger_pull_Steam",
			"joystick_righttrigger_click_Steam",
			"joystick_leftstick_move_Steam",
			"joystick_leftstick_click_Steam",
			"joystick_leftstick_dpadnorth_Steam",
			"joystick_leftstick_dpadsouth_Steam",
			"joystick_leftstick_dpadwest_Steam",
			"joystick_leftstick_dpadeast_Steam",
			"joystick_gyro_move_Steam",
			"joystick_gyro_pitch_Steam",
			"joystick_gyro_yaw_Steam",
			"joystick_gyro_roll_Steam",
		};

		static const float DISPLAY_DURATION = 2000.0f;
		int Index = (int)(Platform::Get()->Get_Ticks() / DISPLAY_DURATION) % count;

		if (origins[Index] >= 0 && origins[Index] <= k_EControllerActionOrigin_Count)
		{
			return origin_atlas[origins[Index]];
		}
	}

	return "";

}

bool Steam_JoystickState::Is_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Steam_JoystickState::Was_Key_Down(InputBindings::Type type)
{
	return false;
}

bool Steam_JoystickState::Was_Key_Pressed(InputBindings::Type type)
{
	return false;
}

bool Steam_JoystickState::Is_Action_Down(OutputBindings::Type type)
{
	return m_current_state[(int)type];
}

bool Steam_JoystickState::Was_Action_Down(OutputBindings::Type type)
{
	return m_previous_state[(int)type];
}

bool Steam_JoystickState::Was_Action_Pressed(OutputBindings::Type type)
{
	return Is_Action_Down(type) && !Was_Action_Down(type);
}

std::string Steam_JoystickState::Read_Character()
{
	return "";
}

void Steam_JoystickState::Flush_Character_Stack()
{
}

JoystickIconSet::Type Steam_JoystickState::Get_Icon_Set()
{
	return JoystickIconSet::Steam;
}

std::string Steam_JoystickState::Get_UniqueID()
{
	return StringHelper::Format("Steam_%i", (int)reinterpret_cast<char*>(m_handle));
}

float Steam_JoystickState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

bool Steam_JoystickState::Show_Binding_Screen()
{
	return SteamController()->ShowBindingPanel(m_handle);
}

bool Steam_JoystickState::ShowInputDialog(std::string ExistingText, std::string Description, int MaxLength)
{
	return SteamUtils()->ShowGamepadTextInput(k_EGamepadTextInputModeNormal, 
		k_EGamepadTextInputLineModeSingleLine, 
		Description.c_str(), 
		MaxLength, 
		ExistingText.c_str()
	);
}

std::string Steam_JoystickState::GetInputDialogResult()
{
	int Len = SteamUtils()->GetEnteredGamepadTextLength();
	char* Buffer = new char[Len + 1];
	memset(Buffer, 0, Len + 1);

	SteamUtils()->GetEnteredGamepadTextInput(Buffer, Len + 1);

	std::string Result = Buffer;
	delete Buffer;

	return Result;
}

bool Steam_JoystickState::IsInputDialogActive()
{
	return OnlinePlatform::Get()->Is_Dialog_Open();
}