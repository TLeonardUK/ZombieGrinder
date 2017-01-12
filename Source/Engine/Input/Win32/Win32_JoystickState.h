// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _ENGINE_INPUT_WIN32_JOYSTICKSTATE_
#define _ENGINE_INPUT_WIN32_JOYSTICKSTATE_

#include "Engine/Input/Input.h"
#include "Engine/Input/JoystickState.h"
#include "Engine/Engine/FrameTime.h"

#include <float.h>
#include <windows.h>
#include <Xinput.h>
#include <dinput.h>
#include <wbemidl.h>
#include <oleauto.h>
//#include <wmsstd.h>

//#define OPT_DISABLE_XINPUT 1

class Win32_Input;
class Win32_JoystickState;

typedef struct
{
	unsigned long						eventCount;
	WORD                                wButtons;
	BYTE                                bLeftTrigger;
	BYTE                                bRightTrigger;
	SHORT                               sThumbLX;
	SHORT                               sThumbLY;
	SHORT                               sThumbRX;
	SHORT                               sThumbRY;
} XINPUT_SECRET_STATE;

class Win32_Joystick
{
public:
	std::string m_name;
	GUID		m_guid;
	GUID		m_guid_product;
	bool		m_connected;
	bool		m_was_connected;
	int			m_preferred_input_index;
	std::string	m_guid_str;
	std::string	m_guid_product_str;

	Win32_JoystickState* m_joystick_state;

public:
	virtual void Refresh() = 0;
	virtual void Tick(const FrameTime& time) = 0;
	virtual void Set_Vibration(float leftMotor, float rightMotor) = 0;
	virtual bool Get_Key_State(InputBindings::Type type) = 0;
	virtual bool Is_Connected();
};

class Win32_Joystick_XInput : public Win32_Joystick
{
private:
	friend class Win32_JoystickManager;

	int m_xinput_source_index;
	XINPUT_STATE m_state;
	XINPUT_SECRET_STATE m_secret_state;
	XINPUT_CAPABILITIES m_caps;

	float m_deadzone;

	int m_connection_frames;
	bool m_disable_tick;

	float m_vibration_left;
	float m_vibration_right;

public:
	Win32_Joystick_XInput();

	virtual void Refresh();
	virtual void Tick(const FrameTime& time);
	virtual void Set_Vibration(float leftMotor, float rightMotor);
	virtual bool Get_Key_State(InputBindings::Type type);

};

struct Win32_Joystick_DirectInput_Button
{
public:
	std::string name;
	GUID		guid;
	DWORD		type;
	int			offset;
	BYTE		value;
	int			internal_offset;
};

struct Win32_Joystick_DirectInput_POV
{
public:
	std::string name;
	GUID		guid;
	DWORD		type;
	int			offset;
	DWORD		value;
	int			internal_offset;
};

struct Win32_Joystick_DirectInput_Axis
{
public:
	std::string name;
	GUID		guid;
	DWORD		type;
	int			offset;
	LONG		min;
	LONG		max;
	LONG		value;
	int			internal_offset;
};

class Win32_Joystick_DirectInput : public Win32_Joystick
{
private:
	friend class Win32_JoystickManager;

	bool m_device_init;
	
	IDirectInputDevice8* m_device;

	DIDATAFORMAT m_data_format;

	float m_deadzone;

	std::vector<Win32_Joystick_DirectInput_Button> m_buttons;
	std::vector<Win32_Joystick_DirectInput_POV> m_povs;
	std::vector<Win32_Joystick_DirectInput_Axis> m_axes;

	bool m_aquired;

	char* m_state;

	void Sort_Input_Objects_To_SDL();
	static bool Sort_Buttons_To_SDL(const Win32_Joystick_DirectInput_Button& a, const Win32_Joystick_DirectInput_Button& b);
	static bool Sort_Axes_To_SDL(const Win32_Joystick_DirectInput_Axis& a, const Win32_Joystick_DirectInput_Axis& b);
	static bool Sort_Hats_To_SDL(const Win32_Joystick_DirectInput_POV& a, const Win32_Joystick_DirectInput_POV& b);

private:
	static BOOL CALLBACK Object_Enum_Callback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);
	static BOOL CALLBACK Effect_Enum_Callback(LPCDIEFFECTINFO pdei, LPVOID pvRef);

	HRESULT Construct_Data_Format();

	bool Get_Physical_Key_State(JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis);

public:
	Win32_Joystick_DirectInput();
	~Win32_Joystick_DirectInput();

	virtual void Refresh();
	virtual void Tick(const FrameTime& time);
	virtual void Set_Vibration(float leftMotor, float rightMotor);
	virtual bool Get_Key_State(InputBindings::Type type);

};

class Win32_JoystickScannedDevice
{
public:
	std::string name;
	GUID		guid;
	bool		is_xinput;
	GUID		guid_product;
};

class Win32_JoystickManager : 
	public Singleton<Win32_JoystickManager>
{
private:
	friend class Win32_JoystickState;

	std::vector<Win32_Joystick*>	m_known_joysticks;
	IDirectInput8*					m_direct_input;

	std::vector<Win32_JoystickScannedDevice>	m_scanned_devices;
	bool										m_scanning_generic;

private:
	static BOOL CALLBACK Device_Enum_Callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

public:
	Win32_JoystickManager();
	~Win32_JoystickManager();

	IDirectInput8* Get_DXI();

	void Tick(const FrameTime& time);
	bool Is_XInput(const GUID *guid);
	void Scan_For_Devices();

	std::vector<JoystickState*> Get_Known_Joysticks();

};

class Win32_JoystickState : public JoystickState
{
	MEMORY_ALLOCATOR(Win32_JoystickState, "Input");

private:
	friend class Win32_Input;
	
	Win32_Joystick* m_joystick;

	bool m_has_input;

	bool m_previous_state[InputBindings::COUNT];
	bool m_current_state[InputBindings::COUNT];

	float m_time_since_last_input;

private:

	bool Has_Input();

public:
	Win32_JoystickState(Win32_Joystick* joystick);
	~Win32_JoystickState();

	void Tick			(const FrameTime& time);

	void Set_Vibration	(float leftMotor, float rightMotor);

	bool Is_Key_Down	(InputBindings::Type type);
	bool Was_Key_Down	(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);

	JoystickIconSet::Type Get_Icon_Set();

	std::string Get_UniqueID();

	float Time_Since_Last_Input();

	bool Is_Connected	();

	std::string Read_Character();
	void Flush_Character_Stack();

};

#endif

#endif