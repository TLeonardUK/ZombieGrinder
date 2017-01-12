// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Steam/Steam_Input.h"
#include "Engine/Input/Null/Null_KeyboardState.h"
#include "Engine/Input/Null/Null_MouseState.h"

#include "Engine/Profiling/ProfilingManager.h"

#include "Generic/Helper/VectorHelper.h"

Steam_Input::Steam_Input()
{
	m_mouse_state = new Null_MouseState(0);
	m_keyboard_state = new Null_KeyboardState(0);

#ifndef DEDICATED_SERVER_BUILD
	if (!SteamController()->Init())
	{
		DBG_LOG("[Steam Controller] Failed to initialize steam controller api.");
	}
	else
	{
		DBG_LOG("[Steam Controller] Initialized steam controller api.");		
	}
#endif
}

Steam_Input::~Steam_Input()
{
#ifndef DEDICATED_SERVER_BUILD
	SteamController()->Shutdown();
#endif
}

void Steam_Input::Tick(const FrameTime& time)
{
#ifndef DEDICATED_SERVER_BUILD
	for (std::vector<Steam_JoystickState*>::iterator iter = m_joysticks.begin(); iter != m_joysticks.end(); iter++)
	{
		(*iter)->Tick(time);
	}


	// Cheap enough to do every frame with steam. If we just do it based on PNP events 
	// then we end up not finding them (steam controllers seem to get added after event).
	Scan_For_Devices();

	{
	//	PROFILE_SCOPE("Update Bindings");
	//	Update_Bindings();
	}
#endif
}

MouseState* Steam_Input::Get_Mouse_State()
{
	return m_mouse_state;
}

KeyboardState* Steam_Input::Get_Keyboard_State()
{
	return m_keyboard_state;
}

void Steam_Input::Scan_For_Devices()
{
#ifndef DEDICATED_SERVER_BUILD
	ControllerHandle_t handles[STEAM_CONTROLLER_MAX_COUNT];
	int controllers = SteamController()->GetConnectedControllers(handles);

	// Disconnect old controllers.
	for (std::vector<Steam_JoystickState*>::iterator iter = m_joysticks.begin(); iter != m_joysticks.end(); iter++)
	{
		Steam_JoystickState* state = *iter;

		bool connected = false;

		for (int i = 0; i < controllers; i++)
		{
			if (handles[i] == state->m_handle)
			{
				connected = true;
				break;
			}
		}

		if (!connected)
		{
			DBG_LOG("[Steam Controller] Steam controller %llu has disconncted.", state->m_handle);
		}
		state->m_connected = connected;
	}

	// Add new controllers.
	for (int i = 0; i < controllers; i++)
	{
		bool bFound = false;

		for (std::vector<Steam_JoystickState*>::iterator iter = m_joysticks.begin(); iter != m_joysticks.end(); iter++)
		{
			Steam_JoystickState* state = *iter;
			if (state->m_handle == handles[i])
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			Steam_JoystickState* state = new Steam_JoystickState(handles[i]);
			state->m_connected = true;

			DBG_LOG("[Steam Controller] Steam controller %llu has disconncted.", state->m_handle);

			m_joysticks.push_back(state);
		}
	}
#endif
}

std::vector<JoystickState*> Steam_Input::Get_Known_Joysticks()
{
	return VectorHelper::Cast<JoystickState*, Steam_JoystickState*>(m_joysticks);
}
