// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _ENGINE_INPUT_MACOS_JOYSTICKSTATE_
#define _ENGINE_INPUT_MACOS_JOYSTICKSTATE_

//#define MACOS_FF_SUPPORT

#include "Engine/Input/JoystickState.h"
#include "Engine/Engine/FrameTime.h"

#ifdef MACOS_FF_SUPPORT
#include <ForceFeedback/ForceFeedback.h>
#endif

#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>

#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/hid/IOHIDKeys.h>

class MacOS_Input;

struct JoystickDataState
{
	enum
	{
		max_axis	 = 255,
		max_hats	 = 255,
		max_buttons	 = 255
	};

	s16		button	[max_buttons];
	float	axis	[max_axis];
	float	hat     [max_hats];
};

struct JoystickProperties
{
	IOHIDElementCookie cookie;
	long sort_value;
	long minimum;
	long maximum;
	int used;
	int coef[3];
};

struct JoystickCaps
{
	enum
	{
		max_axis	 = 255,
		max_hats	 = 255,
		max_buttons	 = 255,
	};

	char					name[256];
	char					guid[256];
	IOHIDDeviceInterface**	device;
	IOHIDDeviceRef			device_ref;

	std::string 			guid_product_str;

#ifdef MACOS_FF_SUPPORT
	FFDeviceObjectReference	ffdevice;
	DWORD					ffcustomparamsaxis[2];
	LONG					ffcustomparamsdirection[2];
	FFPERIODIC				ffcustomparams;
	FFEFFECT				ffparams;
	FFEffectObjectReference	ffeffect;
	FFCAPABILITIES			ffcaps;
	bool					ffenabled;
#endif

	u8						button_count;
	u8						axis_count;
	u8						hat_count;

	JoystickProperties button_properties[max_buttons];
	JoystickProperties axis_properties[max_axis];
	JoystickProperties hat_properties[max_hats];

	std::vector<IOHIDElementCookie> element_cookies;
};

class MacOS_JoystickState : public JoystickState
{
	MEMORY_ALLOCATOR(MacOS_JoystickState, "Input");

private:
	friend class MacOS_Input;
	
	int m_source_index;
	float m_deadzone;

	bool m_has_existed;

	float m_time_since_last_input;
	int m_connection_frames;
	bool m_running;
	bool m_connected;

	JoystickDataState m_current_state;
	JoystickDataState m_previous_state;
	JoystickCaps	  m_caps;

#ifdef MACOS_FF_SUPPORT
	float m_vibration_left;
	float m_vibration_right;

	double m_last_vibration_start;

	bool m_vibration_event_uploaded;
	float m_vibration_left_previous;
	float m_vibration_right_previous;

	int m_test_dir;
#endif

private:
	bool Has_Input();
	bool Get_Physical_Key_State(JoystickDataState& info, JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis);
	bool Get_State(JoystickDataState& info, InputBindings::Type type);
	bool Has_Axis(int axis);
	JoystickDataState Read_State();

#ifdef MACOS_FF_SUPPORT
	void UploadVibrationEvent(float left, float right);
	void StartVibrationEvent();
#endif

public:
	MacOS_JoystickState(int source_index);

	void Set_Vibration(float leftMotor, float rightMotor);

	void Tick(const FrameTime& time);

	bool Is_Key_Down(InputBindings::Type type);
	bool Was_Key_Down(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);

	JoystickIconSet::Type Get_Icon_Set();

	std::string Get_UniqueID();

	float Time_Since_Last_Input();

	bool Is_Connected();
	
	std::string Read_Character();
	void Flush_Character_Stack();

	static std::vector<JoystickState*> Get_All_Joysticks();
	static std::vector<JoystickState*> Get_Known_Joysticks();
	static void Scan_For_Devices();

};

#endif

#endif