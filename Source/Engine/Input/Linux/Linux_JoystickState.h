// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _ENGINE_INPUT_LINUX_JOYSTICKSTATE_
#define _ENGINE_INPUT_LINUX_JOYSTICKSTATE_

#include "Engine/Input/Input.h"
#include "Engine/Input/JoystickState.h"
#include "Engine/Engine/FrameTime.h"

#include "Generic/Threads/Thread.h"

#include <stdio.h>
#include <float.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <limits.h>        
#include <linux/joystick.h>
#include <string>

class Linux_Input;

struct JoystickDataState
{
	enum
	{
		max_axis	 = 128,
		max_hats	 = 128,
		max_buttons	 = 255
	};

	s16		button	[max_buttons];
	float	axis	[max_axis];
	float	hat_axis[max_axis];
};

struct JoystickProperties
{
	struct input_absinfo absinfo;
	int used;
	int coef[3];
};

struct JoystickCaps
{
	enum
	{
		max_axis	 = 128,
		max_hats	 = 128,
		max_buttons	 = 255,
	};

	char	name[256];
	char	guid[256];
	u8		buttons;
	u8		axes;

	int 	axis_map[max_axis];
	int 	button_map[KEY_MAX];
	int 	hat_map[max_hats];

	int 	button_map_size;
	int 	axis_map_size;
	int 	hat_map_size;

	JoystickProperties button_properties[max_buttons];
	JoystickProperties axis_properties[max_axis];
	JoystickProperties hat_properties[max_hats];
};

class Linux_JoystickState : public JoystickState
{
	MEMORY_ALLOCATOR(Linux_JoystickState, "Input");

private:
	friend class Linux_Input;
	
	int m_source_index;
	float m_deadzone;

	float m_time_since_last_input;

	int m_connection_frames;

	bool m_has_existed;

	bool m_connected;

	bool m_running;

	bool m_is_vmware;

	char m_mount_point[64];
	int m_file_handle;

	std::string m_guid_product_str;

	JoystickDataState m_working_state;
	JoystickDataState m_current_state;
	JoystickDataState m_previous_state;
	JoystickCaps  m_caps;

	float m_vibration_left;
	float m_vibration_right;

	bool m_vibration_event_uploaded;
	int m_vibration_event;
	float m_vibration_left_previous;
	float m_vibration_right_previous;

	int m_test_dir;

	input_event m_event;
	int m_event_bytes_read;

	double m_last_vibration_start;

	enum 
	{
		play_event_time_length = 60 * 1000
	};

private:
	void Copy_State();

	void Update_State();
	
	void StartVibrationEvent();
	void UploadVibrationEvent(float left, float right);
	void DeleteVibrationEvent();

	bool Get_Physical_Key_State(JoystickDataState& info, JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis);
	bool Get_State(JoystickDataState& info, InputBindings::Type type);

	int CorrectAxis(int which, int value);

public:
	Linux_JoystickState(int source_index);
	~Linux_JoystickState();

	void Tick(const FrameTime& time);

	void Set_Vibration(float leftMotor, float rightMotor);

	bool Is_Key_Down(InputBindings::Type type);
	bool Was_Key_Down(InputBindings::Type type);
	bool Was_Key_Pressed(InputBindings::Type type);

	JoystickIconSet::Type Get_Icon_Set();

	std::string Get_UniqueID();

	bool Has_Input();
	
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
