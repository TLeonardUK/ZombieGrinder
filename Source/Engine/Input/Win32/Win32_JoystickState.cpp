// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Engine/Input/Win32/Win32_JoystickState.h"
#include "Engine/Display/Win32/Win32_GfxDisplay.h"

#include "Engine/Platform/Platform.h"

#include "Generic/Helper/PersistentLogHelper.h"

#include "Generic/Helper/StringHelper.h"

#include "public/steam/steam_api.h"

// A lot of the directinput code in here is based of the z-doom source code available here:
// https://raw.githubusercontent.com/rheit/zdoom/master/src/win32/i_dijoy.cpp

//#define DIRECTINPUT_VERSION 0x0800

#define XINPUT_DISABLE_SECRET_GUIDE 1

// returns 0 on success, 1167 on not connected. Might be others.
#define XINPUT_GAMEPAD_SECRET_GUIDE 0x0400

#ifndef XINPUT_DISABLE_SECRET_GUIDE
typedef  int(__stdcall *XInputGetSecretStateFuncPtr) (int, XINPUT_SECRET_STATE*);
XInputGetSecretStateFuncPtr XInputGetSecretState = NULL;
#endif

bool Win32_Joystick::Is_Connected()
{
	return m_connected;
}

Win32_Joystick_XInput::Win32_Joystick_XInput()
	: m_deadzone(0.5f)
	, m_vibration_left(0.0f)
	, m_vibration_right(0.0f)
	, m_xinput_source_index(0)
	, m_connection_frames(0)
	, m_disable_tick(false)
{
}

void Win32_Joystick_XInput::Refresh()
{
	m_disable_tick = false;
}

void Win32_Joystick_XInput::Tick(const FrameTime& time)
{
	if (m_disable_tick)
	{
		return;
	}

	int result = -1;
	
	result = XInputGetState(m_xinput_source_index, &m_state);

	if (result == ERROR_SUCCESS && m_state.dwPacketNumber > 0)
	{
#ifndef XINPUT_DISABLE_SECRET_GUIDE
		if (XInputGetSecretState)
		{
			XInputGetSecretState(m_xinput_source_index, &m_secret_state);
		}
#endif

		if (m_connection_frames == 0)
		{
			// Get capabilities.
			result = XInputGetCapabilities(m_xinput_source_index, XINPUT_FLAG_GAMEPAD, &m_caps);
			if (result != ERROR_SUCCESS)
			{
				DBG_LOG("[Joystick] Recieved error code 0x%08x to XInputGetCapabilities after reading a valid state from XInputGetState.", result);
			}
		}

		m_connection_frames++;

		// Update vibration.
		XINPUT_VIBRATION vibs;
		vibs.wLeftMotorSpeed = (int)(m_vibration_left * 65535);
		vibs.wRightMotorSpeed = (int)(m_vibration_right * 65535);
		XInputSetState(m_xinput_source_index, &vibs);
	}
	else
	{
		m_connection_frames = 0;
		m_disable_tick = (result != ERROR_SUCCESS);
	}

	// Flag if we are connected.
	m_connected = (m_connection_frames > 0);
}

void Win32_Joystick_XInput::Set_Vibration(float leftMotor, float rightMotor)
{
	m_vibration_left = leftMotor;
	m_vibration_right = rightMotor;
}

bool Win32_Joystick_XInput::Get_Key_State(InputBindings::Type type)
{
	WORD   wButtons;
	BYTE   bLeftTrigger;
	BYTE   bRightTrigger;
	SHORT  sThumbLX;
	SHORT  sThumbLY;
	SHORT  sThumbRX;
	SHORT  sThumbRY;

#ifndef XINPUT_DISABLE_SECRET_GUIDE
	if (XInputGetSecretState == NULL)
	{
#endif
		wButtons = m_state.Gamepad.wButtons;
		bLeftTrigger = m_state.Gamepad.bLeftTrigger;
		bRightTrigger = m_state.Gamepad.bRightTrigger;
		sThumbLX = m_state.Gamepad.sThumbLX;
		sThumbLY = m_state.Gamepad.sThumbLY;
		sThumbRX = m_state.Gamepad.sThumbRX;
		sThumbRY = m_state.Gamepad.sThumbRY;
#ifndef XINPUT_DISABLE_SECRET_GUIDE
	}
	else
	{
		wButtons = m_secret_state.wButtons;
		bLeftTrigger = m_secret_state.bLeftTrigger;
		bRightTrigger = m_secret_state.bRightTrigger;
		sThumbLX = m_secret_state.sThumbLX;
		sThumbLY = m_secret_state.sThumbLY;
		sThumbRX = m_secret_state.sThumbRX;
		sThumbRY = m_secret_state.sThumbRY;
	}
#endif

	float x_axis = -1.0f + (2.0f * ((sThumbLX + 32768.0f) / 65356.0f));
	float y_axis = -(-1.0f + (2.0f * ((sThumbLY + 32768.0f) / 65356.0f)));
	float u_axis = -1.0f + (2.0f * ((sThumbRX + 32768.0f) / 65356.0f));
	float r_axis = -(-1.0f + (2.0f * ((sThumbRY + 32768.0f) / 65356.0f)));
	float v_axis = 0.0f;
	float dpad = -1.0f;

	float left_axis = (bLeftTrigger / 255.0f);
	float right_axis = (bRightTrigger / 255.0f);

	if (wButtons & XINPUT_GAMEPAD_DPAD_UP)
	{
		dpad = 0.0f;
	}
	if (wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
	{
		dpad = 0.5f;
	}
	if (wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
	{
		dpad = 0.25f;
	}
	if (wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
	{
		dpad = 0.75f;
	}

	if (type == InputBindings::Joystick_Axis2_Pos)			return x_axis > 0 && fabs(x_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis2_Neg)		return x_axis < 0 && fabs(x_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis1_Pos)		return y_axis > 0 && fabs(y_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis1_Neg)		return y_axis < 0 && fabs(y_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis5_Pos)		return left_axis > 0 && fabs(left_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis6_Pos)		return right_axis > 0 && fabs(right_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis3_Pos)		return r_axis > 0 && fabs(r_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis3_Neg)		return r_axis < 0 && fabs(r_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis4_Pos)		return u_axis > 0 && fabs(u_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Axis4_Neg)		return u_axis < 0 && fabs(u_axis) > m_deadzone;
	else if (type == InputBindings::Joystick_Hat1_Right)	return (dpad == 0.25f);
	else if (type == InputBindings::Joystick_Hat1_Left)		return (dpad == 0.75f);
	else if (type == InputBindings::Joystick_Hat1_Down)		return (dpad == 0.5f);
	else if (type == InputBindings::Joystick_Hat1_Up)		return (dpad == 0.0f);
	else if (type >= InputBindings::Joystick_Button1 && type <= InputBindings::Joystick_Button32)
	{
		if (type == InputBindings::Joystick_Button1)  return (wButtons & XINPUT_GAMEPAD_A) != 0;
		if (type == InputBindings::Joystick_Button2) return (wButtons & XINPUT_GAMEPAD_B) != 0;
		if (type == InputBindings::Joystick_Button3) return (wButtons & XINPUT_GAMEPAD_X) != 0;
		if (type == InputBindings::Joystick_Button4) return (wButtons & XINPUT_GAMEPAD_Y) != 0;
		if (type == InputBindings::Joystick_Button5) return (wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
		if (type == InputBindings::Joystick_Button6) return (wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
		if (type == InputBindings::Joystick_Button7) return (wButtons & XINPUT_GAMEPAD_BACK) != 0;
		if (type == InputBindings::Joystick_Button8) return (wButtons & XINPUT_GAMEPAD_START) != 0;
		if (type == InputBindings::Joystick_Button9) return (wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
		if (type == InputBindings::Joystick_Button10) return (wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
		if (type == InputBindings::Joystick_Button11) return (wButtons & XINPUT_GAMEPAD_SECRET_GUIDE) != 0;
		return false;
	}

	return false;
}

Win32_Joystick_DirectInput::Win32_Joystick_DirectInput()
	: m_device_init(false)
	, m_device(NULL)
	, m_aquired(false)
	, m_state(NULL)
	, m_deadzone(0.5f)
{

}

Win32_Joystick_DirectInput::~Win32_Joystick_DirectInput()
{
	if (m_state)
	{
		SAFE_DELETE(m_state);
	}
	if (m_device)
	{
		if (m_aquired)
		{
			m_device->Unacquire();
			m_aquired = false;
		}

		m_device->Release();
		m_device = NULL;
	}
}

void Win32_Joystick_DirectInput::Refresh()
{
}

void Win32_Joystick_DirectInput::Sort_Input_Objects_To_SDL()
{
	std::sort(m_axes.begin(), m_axes.end(), &Sort_Axes_To_SDL);
	std::sort(m_buttons.begin(), m_buttons.end(), &Sort_Buttons_To_SDL);
	std::sort(m_povs.begin(), m_povs.end(), &Sort_Hats_To_SDL);
}

bool Win32_Joystick_DirectInput::Sort_Buttons_To_SDL(const Win32_Joystick_DirectInput_Button& a, const Win32_Joystick_DirectInput_Button& b)
{
	return a.internal_offset < b.internal_offset;
}

bool Win32_Joystick_DirectInput::Sort_Axes_To_SDL(const Win32_Joystick_DirectInput_Axis& a, const Win32_Joystick_DirectInput_Axis& b)
{
	return a.internal_offset < b.internal_offset;
}

bool Win32_Joystick_DirectInput::Sort_Hats_To_SDL(const Win32_Joystick_DirectInput_POV& a, const Win32_Joystick_DirectInput_POV& b)
{
	return a.internal_offset < b.internal_offset;
}

void Win32_Joystick_DirectInput::Tick(const FrameTime& time)
{
	if (!m_device_init)
	{
		m_device_init = true;
		m_aquired = false;

		bool bSuccess = false;

		DBG_LOG("[Joystick] Creating device for DI joystick.");

		IDirectInput8* dxi = Win32_JoystickManager::Get()->Get_DXI();
		HRESULT res = dxi->CreateDevice(m_guid, &m_device, NULL);
		if (SUCCEEDED(res))
		{
			DBG_LOG("[Joystick] Successfully created DI joystick device.");

			DBG_LOG("[Joystick] Enumerating joystick objects.");
			res = m_device->EnumObjects(Object_Enum_Callback, this, DIDFT_ABSAXIS | DIDFT_BUTTON | DIDFT_POV);

			if (SUCCEEDED(res))
			{
				// Resort joystick objects to match SDL's, as thats where we get our bindings from!
				Sort_Input_Objects_To_SDL();

				DBG_LOG("[Joystick] Enumerating joystick effects.");
				
				res = m_device->EnumEffects(Effect_Enum_Callback, this, DIEFT_ALL);
				if (SUCCEEDED(res))
				{
					DBG_LOG("[Joystick] Successfully enumerated joystick effects.");

					DBG_LOG("[Joystick] Constructing joystick data format.");

					res = Construct_Data_Format();

					if (SUCCEEDED(res))
					{
						DBG_LOG("[Joystick] Successfully created joystick data format.");

						HWND hwnd = static_cast<Win32_GfxDisplay*>(GfxDisplay::Get())->Get_Window_Handle();

						DBG_LOG("[Joystick] Setting cooperative level for joystick.");
						res = m_device->SetCooperativeLevel(hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

						if (SUCCEEDED(res))
						{
							DBG_LOG("[Joystick] Successfully set cooperative level for joystick.");

							DBG_LOG("[Joystick] Acquiring device.");

							bSuccess = true;

							res = m_device->Acquire();
							if (SUCCEEDED(res))
							{
								m_aquired = true;
								DBG_LOG("[Joystick] Successfully acquired joystick.");
							}
							else
							{
								DBG_LOG("[Joystick] Failed to acquire joystick with error 0x%08x.", res);
							}
						}
						else
						{
							DBG_LOG("[Joystick] Failed setting cooperative level for joystick 0x%08x.", res);
						}
					}
					else
					{
						DBG_LOG("[Joystick] Failed creating joystick data format with error 0x%08x.", res);
					}
				}
				else
				{
					DBG_LOG("[Joystick] Failed to enumerate effects 0x%08x.", res);
				}
			}
			else
			{
				DBG_LOG("[Joystick] Failed enumerating joystick objects with error 0x%08x.", res);
			}
		}
		else
		{
			DBG_LOG("[Joystick] Failed creating DI joystick device with error 0x%08x.", res);
		}

		if (!bSuccess)
		{
			if (m_device)
			{
				if (m_aquired)
				{
					m_device->Unacquire();
					m_aquired = false;
				}

				m_device->Release();
				m_device = NULL;
			}
		}
	}

	if (m_device && m_connected)
	{
		HRESULT res = m_device->Poll();
		if (res == DIERR_INPUTLOST || res == DIERR_NOTACQUIRED)
		{
			res = m_device->Acquire();
		}

		if (SUCCEEDED(res))
		{
			res = m_device->GetDeviceState(m_data_format.dwDataSize, m_state);
			if (SUCCEEDED(res))
			{
				for (unsigned int i = 0; i < m_axes.size(); i++)
				{
					Win32_Joystick_DirectInput_Axis& info = m_axes[i];
					info.value = *(LONG*)(m_state + info.offset);

					/*if (fabs(info.value) > 0.1f)
					{
						DBG_ONSCREEN_LOG(StringHelper::Hash(StringHelper::Format("axis_%i", i).c_str()), Color::Green, 1.0f, "Axes{%i}=%f", i, (float)info.value);
					}*/

				}
				for (unsigned int i = 0; i < m_povs.size(); i++)
				{
					Win32_Joystick_DirectInput_POV& info = m_povs[i];
					info.value = *(DWORD *)(m_state + info.offset);

					/*if (fabs(info.value) > 0.1f)
					{
						DBG_ONSCREEN_LOG(StringHelper::Hash(StringHelper::Format("pov_%i", i).c_str()), Color::Green, 1.0f, "POV{%i}=%f", i, (float)info.value);
					}*/
				}
				for (unsigned int i = 0; i < m_buttons.size(); i++)
				{
					Win32_Joystick_DirectInput_Button& info = m_buttons[i];
					info.value = *(BYTE *)(m_state + info.offset);

					/*if (info.value > 0)
					{
						DBG_ONSCREEN_LOG(StringHelper::Hash(StringHelper::Format("pov_%i", i).c_str()), Color::Green, 1.0f, "Button{%i}=%f", i, (float)info.value);
					}*/
				}
			}
			else
			{
				DBG_LOG("[Joystick] Failed to get joystick device state 0x%08x.", res);
			}

			/*
			std::string msg = "Axis: ";
			for (int i = 0; i < 8; i++)
			{
				Win32_Joystick_DirectInput_Axis& axis = m_axes[i];
				float val = (axis.value - axis.min) * 2.0f / (axis.max - axis.min) - 1.0f;
				msg = StringHelper::Format("%s [%i]=%.2f", msg.c_str(), i, val);
			}
			DBG_LOG("%s", msg.c_str());
			*/
		}
		else
		{
		//	DBG_LOG("[Joystick] Failed to poll joystick device 0x%08x.", res);
		}
	}
}

HRESULT Win32_Joystick_DirectInput::Construct_Data_Format()
{
	DIOBJECTDATAFORMAT *objects;
	DWORD numobjs;
	DWORD nextofs;
	unsigned i;

	objects = new DIOBJECTDATAFORMAT[m_axes.size() + m_povs.size() + m_buttons.size()];
	numobjs = nextofs = 0;

	for (i = 0; i < m_axes.size(); ++i)
	{
		objects[i].pguid = &m_axes[i].guid;
		objects[i].dwOfs = m_axes[i].offset = nextofs;
		objects[i].dwType = m_axes[i].type;
		objects[i].dwFlags = 0;
		nextofs += sizeof(LONG);
	}
	numobjs = i;

	for (i = 0; i < m_povs.size(); ++i)
	{
		objects[numobjs + i].pguid = &m_povs[i].guid;
		objects[numobjs + i].dwOfs = m_povs[i].offset = nextofs;
		objects[numobjs + i].dwType = m_povs[i].type;
		objects[numobjs + i].dwFlags = 0;
		nextofs += sizeof(DWORD);
	}
	numobjs += i;

	for (i = 0; i < m_buttons.size(); ++i)
	{
		objects[numobjs + i].pguid = &m_buttons[i].guid;
		objects[numobjs + i].dwOfs = m_buttons[i].offset = nextofs;
		objects[numobjs + i].dwType = m_buttons[i].type;
		objects[numobjs + i].dwFlags = 0;
		nextofs += sizeof(BYTE);
	}
	numobjs += i;

	// Set format
	m_data_format.dwSize = sizeof(DIDATAFORMAT);
	m_data_format.dwObjSize = sizeof(DIOBJECTDATAFORMAT);
	m_data_format.dwFlags = DIDF_ABSAXIS;
	m_data_format.dwDataSize = (nextofs + 3) & ~3;		// Round to the nearest multiple of 4.
	m_data_format.dwNumObjs = numobjs;
	m_data_format.rgodf = objects;

	if (m_state)
	{
		SAFE_DELETE(m_state);
	}
	m_state = new char[m_data_format.dwDataSize];

	return m_device->SetDataFormat(&m_data_format);
}

BOOL CALLBACK Win32_Joystick_DirectInput::Effect_Enum_Callback(LPCDIEFFECTINFO pdei, LPVOID pvRef)
{
	Win32_Joystick_DirectInput* joy = (Win32_Joystick_DirectInput*)pvRef;

	DBG_LOG("[Joystick] Device has effect: %s", pdei->tszName);

	// TODO: Do something with these.

	return DIENUM_CONTINUE;
}

BOOL CALLBACK Win32_Joystick_DirectInput::Object_Enum_Callback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
	Win32_Joystick_DirectInput* joy = (Win32_Joystick_DirectInput*)pvRef;

	if (lpddoi->guidType == GUID_Button)
	{
		Win32_Joystick_DirectInput_Button info;
		info.name = lpddoi->tszName;
		info.guid = lpddoi->guidType;
		info.type = lpddoi->dwType;
		info.offset = 0;
		info.value = 0;
		info.internal_offset = DIJOFS_BUTTON(joy->m_buttons.size());

		if (joy->m_buttons.size() < 256)
		{
			joy->m_buttons.push_back(info);
		}
	}
	else if (lpddoi->guidType == GUID_POV)
	{
		Win32_Joystick_DirectInput_POV info;
		info.name = lpddoi->tszName;
		info.guid = lpddoi->guidType;
		info.type = lpddoi->dwType;
		info.offset = 0;
		info.value = 0;
		info.internal_offset = DIJOFS_POV(joy->m_povs.size());

		if (joy->m_povs.size() < 4)
		{
			joy->m_povs.push_back(info);
		}
	}
	else if (lpddoi->guidType == GUID_XAxis ||
			 lpddoi->guidType == GUID_YAxis ||
			 lpddoi->guidType == GUID_ZAxis ||
			 lpddoi->guidType == GUID_RxAxis ||
			 lpddoi->guidType == GUID_RyAxis ||
			 lpddoi->guidType == GUID_RzAxis ||
			 lpddoi->guidType == GUID_Slider)
	{
		DIPROPRANGE diprg;
		Win32_Joystick_DirectInput_Axis info;

		diprg.diph.dwSize = sizeof(diprg);
		diprg.diph.dwHeaderSize = sizeof(diprg.diph);
		diprg.diph.dwObj = lpddoi->dwType;
		diprg.diph.dwHow = DIPH_BYID;
		diprg.lMin = -32768;
		diprg.lMax = 32767;
		joy->m_device->GetProperty(DIPROP_RANGE, &diprg.diph);

		DIPROPDWORD dilong;
		dilong.diph.dwSize = sizeof(dilong);
		dilong.diph.dwHeaderSize = sizeof(dilong.diph);
		dilong.diph.dwObj = lpddoi->dwType;
		dilong.diph.dwHow = DIPH_BYID;
		dilong.dwData = 0;
		joy->m_device->SetProperty(DIPROP_DEADZONE, &dilong.diph);

		/*
		DIPROPCAL dical;
		dical.diph.dwSize = sizeof(DIPROPCAL);
		dical.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dical.diph.dwObj = lpddoi->dwType;
		dical.diph.dwHow = DIPH_BYID;
		dical.lMin = 0;
		dical.lMax = 0;
		dical.lCenter = 0;
		joy->m_device->GetProperty(DIPROP_CALIBRATION, &dical.diph);
		*/
		info.name = lpddoi->tszName;
		info.guid = lpddoi->guidType;
		info.type = lpddoi->dwType;
		info.offset = 0;
		info.min = diprg.lMin;
		info.max = diprg.lMax;
		info.value = 0;
		
		// Based on SDL, thanks chaps.
		if (lpddoi->guidType == GUID_XAxis)
		{
			info.internal_offset = DIJOFS_X;
		}
		else if (lpddoi->guidType == GUID_YAxis)
		{
			info.internal_offset = DIJOFS_Y;
		}
		else if (lpddoi->guidType == GUID_ZAxis)
		{
			info.internal_offset = DIJOFS_Z;
		}
		else if (lpddoi->guidType == GUID_RxAxis)
		{
			info.internal_offset = DIJOFS_RX;
		}
		else if (lpddoi->guidType == GUID_RyAxis)
		{
			info.internal_offset = DIJOFS_RY;
		}
		else if (lpddoi->guidType == GUID_RzAxis)
		{
			info.internal_offset = DIJOFS_RZ;
		}
		else
		{
			// Can't deal with this.
			return DIENUM_CONTINUE;
		}

		joy->m_axes.push_back(info);
	}

	return DIENUM_CONTINUE;
}

void Win32_Joystick_DirectInput::Set_Vibration(float leftMotor, float rightMotor)
{
	// TODO: Force feedback interface plz
}

bool Win32_Joystick_DirectInput::Get_Physical_Key_State(JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis)
{
	if (type == JoystickMappingInputType::Axis)
	{
		if (index >= 0 && index < (int)m_axes.size())
		{
			if (!mapping->Axis_Properties[index].Is_Enabled)
			{
				return false;
			}

			Win32_Joystick_DirectInput_Axis& axis = m_axes[index];

			float val = (axis.value - axis.min) * 2.0f / (axis.max - axis.min) - 1.0f;
			if (!half_axis)
			{
				val = (float)(axis.value - axis.min) / (float)(axis.max - axis.min);
			}
			
			if (inverted)
			{
				return val < 0 && fabs(val) > m_deadzone;
			}
			else
			{
				return val > 0 && fabs(val) > m_deadzone;
			}
		}
	}
	else if (type == JoystickMappingInputType::Hat)
	{
		int hat_index = index / 4;
		int hat_direction = index %  4;

		if (hat_index >= 0 && hat_index < (int)m_povs.size())
		{
			if (!mapping->Hat_Properties[index].Is_Enabled)
			{
				return false;
			}

			float dpad = 0.0f;

			Win32_Joystick_DirectInput_POV& pov = m_povs[hat_index];
			if (pov.value < 0 || pov.value > 36000)
			{
				dpad = -1.0f;
			}
			else
			{
				dpad = pov.value / 36000.0f;
			}

			     if (hat_direction == 0) return (dpad == 0.25f); // Right
			else if (hat_direction == 1) return (dpad == 0.75f); // Left
			else if (hat_direction == 2) return (dpad == 0.5f);  // Up
			else if (hat_direction == 3) return (dpad == 0.0f);  // Down
		}
	}
	else if (type == JoystickMappingInputType::Button)
	{
		if (index >= 0 && index < (int)m_buttons.size())
		{
			if (!mapping->Button_Properties[index].Is_Enabled)
			{
				return false;
			}

			Win32_Joystick_DirectInput_Button& button = m_buttons[index];
			return button.value != 0;
		}
	}

	return false;
}

bool Win32_Joystick_DirectInput::Get_Key_State(InputBindings::Type type)
{
	JoystickMapping* mapping = Input::Get_Joystick_Mapping(m_guid_product_str);

	// Axes
	if	(type > InputBindings::JOYSTICK_AXIS_START && type < InputBindings::JOYSTICK_AXIS_END)
	{
		int axis_value_input = ((type - InputBindings::JOYSTICK_AXIS_START) - 1);
		int axes_index = axis_value_input / 2;
		bool axes_inverted = (axis_value_input % 2) != 0;

		bool half_axis = false;
		if (type == InputBindings::Joystick_Axis1_Neg ||
			type == InputBindings::Joystick_Axis1_Pos ||
			type == InputBindings::Joystick_Axis2_Neg ||
			type == InputBindings::Joystick_Axis2_Pos ||
			type == InputBindings::Joystick_Axis3_Neg ||
			type == InputBindings::Joystick_Axis3_Pos ||
			type == InputBindings::Joystick_Axis4_Neg ||
			type == InputBindings::Joystick_Axis4_Pos)
		{
			half_axis = true;
		}

		// Remap based on mappings.
		JoystickMappingProperties& mapping_props = mapping->Axis_Mappings[axes_index];
		return Get_Physical_Key_State(mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, axes_inverted, half_axis);
	}

	// POVS
	else if (type > InputBindings::JOYSTICK_HAT_START && type < InputBindings::JOYSTICK_HAT_END)
	{
		int hat_value_input = ((type - InputBindings::JOYSTICK_HAT_START) - 1);
		int hat_index = hat_value_input;

		float dpad = -1.0f;

		// Remap based on mappings.
		JoystickMappingProperties& mapping_props = mapping->Hat_Mappings[hat_index];
		return Get_Physical_Key_State(mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, false, false);
	}

	// Buttons
	else if (type >= InputBindings::Joystick_Button1 && type <= InputBindings::Joystick_Button32)
	{
		int button_id = ((type - InputBindings::JOYSTICK_BUTTON_START) - 1);

		// Remap based on mappings.
		JoystickMappingProperties& mapping_props = mapping->Button_Mappings[button_id];
		return Get_Physical_Key_State(mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, false, false);
	}

	return false;
}


Win32_JoystickManager::Win32_JoystickManager()
	: m_direct_input(NULL)
{
	HINSTANCE hInstance = GetModuleHandle(0);

	DBG_LOG("[Joystick] Attempting to create DirectInput device with version 0x%08x", DIRECTINPUT_VERSION);

	HRESULT result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_direct_input, NULL);
	if (SUCCEEDED(result))
	{
		DBG_LOG("[Joystick] Successfully created DirectInput device.");
		Scan_For_Devices();
	}
	else
	{
		DBG_LOG("[Joystick] Failed to create DirectInput device with result 0x%08x.", result);
	}

#ifndef XINPUT_DISABLE_SECRET_GUIDE
	DBG_LOG("[Joystick] Attempting to load secret ordinal functions from xinput library.");
	TCHAR xinput_dll_path[MAX_PATH];
	GetSystemDirectory(xinput_dll_path, sizeof(xinput_dll_path));
	strcat(xinput_dll_path, "\\xinput1_3.dll");

	HINSTANCE xinput_dll = LoadLibrary(xinput_dll_path);
	if (xinput_dll != INVALID_HANDLE_VALUE)
	{
		XInputGetSecretState = (XInputGetSecretStateFuncPtr)GetProcAddress(xinput_dll, (LPCSTR)100); // load ordinal 100
	}
	else
	{
		XInputGetSecretState = NULL;
	}

	if (XInputGetSecretState)
	{
		DBG_LOG("[Joystick] Successfullly loaded secret functions.");
	}
	else
	{
		DBG_LOG("[Joystick] Failed to load secret functions.");
	}
#endif
}

Win32_JoystickManager::~Win32_JoystickManager()
{
	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		SAFE_DELETE(joystick);
	}

	m_known_joysticks.clear();

	if (m_direct_input)
	{
		m_direct_input->Release();
		m_direct_input = NULL;
	}
}

IDirectInput8* Win32_JoystickManager::Get_DXI()
{
	return m_direct_input;
}

void Win32_JoystickManager::Tick(const FrameTime& time)
{
	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		joystick->Tick(time);
	}
}

bool Win32_JoystickManager::Is_XInput(const GUID *guid)
{
#ifdef OPT_DISABLE_XINPUT
	return false;
#endif

	// Based on the code in zdoom.

	bool bIsXInput = false;

	UINT nDevices;
	if (GetRawInputDeviceList(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) != 0)
	{
		goto cleanup;
	}

	RAWINPUTDEVICELIST *devices = new RAWINPUTDEVICELIST[nDevices];
	UINT deviceCount = GetRawInputDeviceList(devices, &nDevices, sizeof(RAWINPUTDEVICELIST));

	if (deviceCount == (UINT)-1)
	{
		goto cleanup;
	}

	for (UINT i = 0; i < deviceCount; ++i)
	{
		// Assumption that all xinput devices will report as generic HID devices.
		if (devices[i].dwType == RIM_TYPEHID)
		{
			RID_DEVICE_INFO rdi;
			UINT cbSize;

			cbSize = rdi.cbSize = sizeof(rdi);

			if ((INT)GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICEINFO, &rdi, &cbSize) >= 0)
			{
				LONG current = MAKELONG(rdi.hid.dwVendorId, rdi.hid.dwProductId);
				LONG needle = (LONG)guid->Data1;
				if (current == needle)
				{
					char name[256];
					UINT namelen = 256;
					UINT reslen;

					reslen = GetRawInputDeviceInfoA(devices[i].hDevice, RIDI_DEVICENAME, name, &namelen);
					if (reslen != (UINT)-1)
					{
						bIsXInput = (strstr(name, "IG_") != NULL); // Any device names beginning with IG_ are xinput devices.
						goto cleanup;
					}
				}
			}
		}
	}

cleanup:
	if (devices)
	{
		SAFE_DELETE(devices);
	}

	return bIsXInput;
}

BOOL CALLBACK Win32_JoystickManager::Device_Enum_Callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	// Based on the code in zdoom, we have a special check for generic controllers because
	// the microsoft sidewinder strategic controller reports itself wrong.

	Win32_JoystickManager* manager = reinterpret_cast<Win32_JoystickManager*>(pvRef);

	if (manager->m_scanning_generic && lpddi->guidProduct.Data1 != MAKELONG(0x45e, 0x0033))
	{
		return DIENUM_CONTINUE;
	}

	Win32_JoystickScannedDevice device;
	device.guid			= lpddi->guidInstance;
	device.name			= lpddi->tszInstanceName;
	device.is_xinput	= manager->Is_XInput(&lpddi->guidProduct);
	device.guid_product = lpddi->guidProduct;

	manager->m_scanned_devices.push_back(device);
	
	return DIENUM_CONTINUE;
}

void Win32_JoystickManager::Scan_For_Devices()
{
	if (!m_direct_input)
	{
		return;
	}

	m_scanned_devices.clear();

	DBG_LOG("[Joystick] Scanning for DirectInput devices...");
	double start = Platform::Get()->Get_Ticks();

	m_scanning_generic = false;
	m_direct_input->EnumDevices(DI8DEVCLASS_GAMECTRL, Device_Enum_Callback, this, DIEDFL_ATTACHEDONLY);

	m_scanning_generic = true;
	m_direct_input->EnumDevices(DI8DEVCLASS_DEVICE, Device_Enum_Callback, this, DIEDFL_ATTACHEDONLY);

	double elapsed = Platform::Get()->Get_Ticks() - start;
	DBG_LOG("[Joystick] Completed scan in %.2f ms, found %i devices.", elapsed, m_scanned_devices.size());

	FrameTime FakeTime(60);

	// If no known joystick, add some dummy xinput ones.
#ifndef OPT_DISABLE_XINPUT
	if (m_known_joysticks.size() == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			Win32_Joystick_XInput* joystick = new Win32_Joystick_XInput();
			joystick->m_was_connected = false;
			joystick->m_connected = false;
			joystick->m_xinput_source_index = i;
			joystick->m_preferred_input_index = i;
			joystick->m_guid.Data1 = i;
			joystick->m_guid.Data2 = 0;
			joystick->m_guid.Data3 = 0;
			joystick->m_guid_product.Data1 = i;
			joystick->m_guid_product.Data2 = 0;
			joystick->m_guid_product.Data3 = 0;
			joystick->m_name = StringHelper::Format("XInput Source %i", i);
			joystick->m_joystick_state = new Win32_JoystickState(joystick);

			unsigned short pid = HIWORD(joystick->m_guid.Data1);
			unsigned char pid_lo = LOBYTE(pid);
			unsigned char pid_hi = HIBYTE(pid);
			unsigned short vid = LOWORD(joystick->m_guid.Data1);
			unsigned char vid_lo = LOBYTE(vid);
			unsigned char vid_hi = HIBYTE(vid);
			joystick->m_guid_str = StringHelper::Format(
				"%02hx%02hx%02hx%02hx%04hx%04hx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
				vid_lo, vid_hi, pid_lo, pid_hi, joystick->m_guid.Data2, joystick->m_guid.Data3,
				joystick->m_guid.Data4[0], joystick->m_guid.Data4[1], joystick->m_guid.Data4[2], joystick->m_guid.Data4[3],
				joystick->m_guid.Data4[4], joystick->m_guid.Data4[5], joystick->m_guid.Data4[6], joystick->m_guid.Data4[7]
				);

			m_known_joysticks.push_back(joystick);
		}
	}
#endif

	// Reset the connected state so we can detect disconnects.
	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		if (dynamic_cast<Win32_Joystick_XInput*>(joystick) != NULL)
		{
			joystick->m_connected = false;
		}
	}

	// Refresh device states.
	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		joystick->Refresh();
		joystick->Tick(FakeTime);
	}

	// Find all new joystick devices.
	for (unsigned int i = 0; i < m_scanned_devices.size(); i++)
	{
		Win32_JoystickScannedDevice& device = m_scanned_devices[i];

		bool bFound = false;
		for (unsigned j = 0; j < m_known_joysticks.size(); j++)
		{
			Win32_Joystick* joystick = m_known_joysticks[j];
			if (joystick->m_guid == device.guid)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound && !device.is_xinput)
		{
			Win32_Joystick* joystick = new Win32_Joystick_DirectInput();

			joystick->m_guid = device.guid;
			joystick->m_guid_product = device.guid_product;

			{
				unsigned short pid = HIWORD(joystick->m_guid.Data1);
				unsigned char pid_lo = LOBYTE(pid);
				unsigned char pid_hi = HIBYTE(pid);
				unsigned short vid = LOWORD(joystick->m_guid.Data1);
				unsigned char vid_lo = LOBYTE(vid);
				unsigned char vid_hi = HIBYTE(vid);
				joystick->m_guid_str = StringHelper::Format(
					"%02hx%02hx%02hx%02hx%04hx%04hx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
					vid_lo, vid_hi, pid_lo, pid_hi, joystick->m_guid.Data2, joystick->m_guid.Data3,
					joystick->m_guid.Data4[0], joystick->m_guid.Data4[1], joystick->m_guid.Data4[2], joystick->m_guid.Data4[3],
					joystick->m_guid.Data4[4], joystick->m_guid.Data4[5], joystick->m_guid.Data4[6], joystick->m_guid.Data4[7]
				);
			}

			{
				// Note: This has been munged slightly to conform to SDL guid's.
				unsigned short pid = HIWORD(device.guid_product.Data1);
				unsigned char pid_lo = LOBYTE(pid);
				unsigned char pid_hi = HIBYTE(pid);
				unsigned short vid = LOWORD(device.guid_product.Data1);
				unsigned char vid_lo = LOBYTE(vid);
				unsigned char vid_hi = HIBYTE(vid);
				joystick->m_guid_product_str = StringHelper::Format(
					"%02hx%02hx%02hx%02hx%04hx%04hx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
					vid_lo, vid_hi, pid_lo, pid_hi, device.guid_product.Data2, device.guid_product.Data3,
					device.guid_product.Data4[0], device.guid_product.Data4[1], device.guid_product.Data4[2], device.guid_product.Data4[3],
					device.guid_product.Data4[4], device.guid_product.Data4[5], device.guid_product.Data4[6], device.guid_product.Data4[7]
				);
			}

			joystick->m_name = device.name;
			joystick->m_connected = true;
			joystick->m_was_connected = true;
			joystick->m_preferred_input_index = -1;
			joystick->m_joystick_state = new Win32_JoystickState(joystick);

			DBG_LOG("[Joystick]  New Device: Name=%s XInput=%i Guid=%s Mapping=%s",
				joystick->m_name.c_str(),
				dynamic_cast<Win32_Joystick_XInput*>(joystick) != NULL ? 1 : 0,
				joystick->m_guid_product_str.c_str(),
				Input::Get_Joystick_Mapping(joystick->m_guid_product_str.c_str())->Name.c_str());

			joystick->Tick(FakeTime);

			m_known_joysticks.push_back(joystick);
		}
	}

	// Find all lost devices.
	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		if (!joystick->m_connected && joystick->m_was_connected)
		{
			DBG_LOG("[Joystick]  Device Disconnected: Name=%s XInput=%i Guid=%s",
				joystick->m_name.c_str(),
				dynamic_cast<Win32_Joystick_XInput*>(joystick) != NULL ? 1 : 0,
				joystick->m_guid_product_str.c_str());
		}
		else if (joystick->m_connected && !joystick->m_was_connected)
		{
			DBG_LOG("[Joystick]  Device Reconnected: Name=%s XInput=%i Guid=%s",
				joystick->m_name.c_str(),
				dynamic_cast<Win32_Joystick_XInput*>(joystick) != NULL ? 1 : 0,
				joystick->m_guid_product_str.c_str());
		}
		joystick->m_was_connected = joystick->m_connected;
	}

}

std::vector<JoystickState*> Win32_JoystickManager::Get_Known_Joysticks()
{
	std::vector<JoystickState*> result;

	for (unsigned int j = 0; j < m_known_joysticks.size(); j++)
	{
		Win32_Joystick* joystick = m_known_joysticks[j];
		result.push_back(joystick->m_joystick_state);
	}

	return result;
}

Win32_JoystickState::Win32_JoystickState(Win32_Joystick* joystick)
	: m_time_since_last_input(0.0f)
	, m_has_input(false)
	, m_joystick(joystick)
{
	memset(m_current_state, 0, sizeof(bool) * InputBindings::COUNT);
	memset(m_previous_state, 0, sizeof(bool) * InputBindings::COUNT);
}

Win32_JoystickState::~Win32_JoystickState()
{
}

void Win32_JoystickState::Set_Vibration(float leftMotor, float rightMotor)
{
	if (m_joystick)
	{
		m_joystick->Set_Vibration(leftMotor, rightMotor);
	}
}

void Win32_JoystickState::Tick(const FrameTime& time)
{
	m_has_input = false;

	for (int i = InputBindings::JOYSTICK_START + 1; i < InputBindings::JOYSTICK_END; i++)
	{
		InputBindings::Type key = (InputBindings::Type)i;

		m_previous_state[i] = m_current_state[i];
		m_current_state[i] = m_joystick ? m_joystick->Get_Key_State(key) : false;

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

bool Win32_JoystickState::Is_Connected()
{
	return m_joystick != NULL && m_joystick->Is_Connected();
}

bool Win32_JoystickState::Has_Input()
{
	return m_has_input;
}

JoystickIconSet::Type Win32_JoystickState::Get_Icon_Set()
{
	if (m_joystick != NULL)
	{
		return Input::Get_Joystick_Mapping(m_joystick->m_guid_product_str)->Icon_Set;
	}
	return JoystickIconSet::Xbox360;
}

std::string Win32_JoystickState::Get_UniqueID()
{
	return m_joystick->m_name + "(" + m_joystick->m_guid_str + ")";
}

bool Win32_JoystickState::Is_Key_Down(InputBindings::Type type)
{
	return m_current_state[(int)type];
}

bool Win32_JoystickState::Was_Key_Down(InputBindings::Type type)
{
	return m_previous_state[(int)type];
}

bool Win32_JoystickState::Was_Key_Pressed(InputBindings::Type type)
{
	return Is_Key_Down(type) && !Was_Key_Down(type);
}

std::string Win32_JoystickState::Read_Character()
{
	// TODO: SteamController text input.
	return "";
}

void Win32_JoystickState::Flush_Character_Stack()
{
	// TODO: SteamController text input.
}

float Win32_JoystickState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

#endif