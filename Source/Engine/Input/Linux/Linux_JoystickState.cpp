// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Engine/Input/Linux/Linux_JoystickState.h"
#include "Engine/Display/Linux/Linux_GfxDisplay.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Platform/Platform.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/Input/Input.h"

#include "Generic/Math/Math.h"

#include <float.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <limits.h>        
#include <linux/joystick.h>

/*
#define JOY_X		0
#define JOY_Y		1
#define JOY_Z		2
#define JOY_R		3
#define JOY_U		4
#define JOY_V		5
#define JOY_YAW		6
#define JOY_PITCH	7
#define JOY_ROLL	8
#define JOY_HAT		9
#define JOY_WHEEL	10
*/

#define test_bit(nr, addr) (((1UL << ((nr) % (sizeof(long) * 8))) & ((addr)[(nr) / (sizeof(long) * 8)])) != 0)
#define NBITS(x) ((((x)-1)/(sizeof(long) * 8))+1)

static bool GRetrievedMountPoints = false;
std::string GMountPoints[MAX_INPUT_SOURCES];

bool Get_Controller_Mount_Point(char* mount_point, int index)
{
	// This whole function is all kinds of ugly. There really has to be 
	// a better way to do this in linux, but after hours of fruitless searching
	// this actually seems the correct way -_-

	if (!GRetrievedMountPoints)
	{
		int pipe = open("/proc/bus/input/devices", O_RDONLY);
		if (pipe >= 0)
		{
			const int max_output = 64 * 1024;
			char* buffer = new char[max_output];

			int bytes_read = 0;
			while (true)
			{
				int bytes = read(pipe, buffer + bytes_read, max_output - bytes_read);
				if (bytes <= 0)
				{
					break;
				}
				bytes_read += bytes;
			}
			buffer[bytes_read + 1] = '\0';

			std::vector<std::string> lines;
			StringHelper::Split(buffer, '\n', lines);
			
			int mount_index = 0;
			std::string mount_handler = "";
			bool mount_has_abs = false;
			bool mount_is_mouse = false;

			int line_index = 0;
			while (line_index <= lines.size())
			{
				std::string line = line_index < lines.size() ? lines[line_index] : "";
				line_index++;

				if (line == "")
				{
					if (mount_handler != "" && mount_has_abs == true && mount_is_mouse == false)
					{
						DBG_LOG("Found Joystick: %s", mount_handler.c_str());
						GMountPoints[mount_index++] = mount_handler;
						if (mount_index >= MAX_INPUT_SOURCES)
						{
							break;
						}
					}
					mount_handler = "";
					mount_has_abs = false;
					mount_is_mouse = false;
				}
				else
				{
					std::vector<std::string> main_split;
					StringHelper::Split(line.c_str() + 3, '=', main_split);

					if (main_split.size() >= 2)
					{
						std::string command = main_split[0];
						std::string values = main_split[1];

						//DBG_LOG("Command='%s' Values='%s'", command.c_str(), values.c_str());

						// Handlers=
						if (command == "Handlers")
						{
							std::vector<std::string> handler_split;
							StringHelper::Split(values.c_str(), ' ', handler_split);

							for (std::vector<std::string>::iterator handler_iter = handler_split.begin(); handler_iter != handler_split.end(); handler_iter++)
							{
								std::string& handler = *handler_iter;
								if (handler.size() > 5 && handler.substr(0, 5) == "event")
								{
									mount_handler = "/dev/input/" + handler;
								}
								if (handler.size() > 5 && handler.substr(0, 5) == "mouse")
								{
									mount_is_mouse = true;
								}
							}
						}
						// Abs=
						else if (command == "ABS")
						{
							mount_has_abs = true;
						}
					}
				}
			}

			close(pipe);
			SAFE_DELETE(buffer);
		}
		else
		{
			DBG_LOG("Failed to open /proc/bus/input/devices, unable to parse joysticks.");
		}

		GRetrievedMountPoints = true;
	}

	std::string point = GMountPoints[index];
	if (point != "")
	{
		strcpy(mount_point, point.c_str());
		return true;
	}
	else
	{
		return false;
	}
}

Linux_JoystickState::Linux_JoystickState(int source_index)
	: m_source_index(source_index)
	, m_deadzone(0.5f)
	, m_time_since_last_input(FLT_MAX)
	, m_connection_frames(0)
	, m_running(true)
	, m_connected(false)
	, m_is_vmware(false)
	, m_vibration_left(0.0f)
	, m_vibration_right(0.0f)
	, m_vibration_left_previous(0.0f)
	, m_vibration_right_previous(0.0f)
	, m_vibration_event_uploaded(false)
	, m_vibration_event(-1)
	, m_file_handle(-1)
	, m_test_dir(0)
	, m_has_existed(false)
{
	memset(&m_working_state, 0, sizeof(JoystickDataState));
	memset(&m_previous_state, 0, sizeof(JoystickDataState));
	memset(&m_current_state, 0, sizeof(JoystickDataState));
	memset(&m_caps, 0, sizeof(m_caps));
}

Linux_JoystickState::~Linux_JoystickState()
{
	if (m_vibration_event_uploaded)
	{
		DeleteVibrationEvent();
	}

	if (m_file_handle >= 0)
	{
		close(m_file_handle);
		m_file_handle = -1;
	}

	m_connected = false;
	m_running = false;
}

void Linux_JoystickState::Copy_State()
{	
	memcpy(&m_previous_state, &m_current_state, sizeof(JoystickDataState));
	memcpy(&m_current_state,  &m_working_state, sizeof(JoystickDataState));
}

void Linux_JoystickState::Set_Vibration(float leftMotor, float rightMotor)
{
	m_vibration_left = leftMotor;
	m_vibration_right = rightMotor;
}

void Linux_JoystickState::DeleteVibrationEvent()
{
	// Stop vibration event.
	struct input_event vib_event;
	vib_event.type = EV_FF;
	vib_event.code = m_vibration_event;
	vib_event.value = 0;
	write(m_file_handle, (const void*)&vib_event, sizeof(vib_event));

	// Delete vibration event.
	int retval = ioctl(m_file_handle, EVIOCRMFF, m_vibration_event);
	if (retval == -1)
	{
		DBG_LOG("EVIOCRMFF Failed: %i", retval);
	}
	m_vibration_event_uploaded = false;
}

void Linux_JoystickState::UploadVibrationEvent(float left, float right)
{
	struct ff_effect play_event;

	play_event.type = FF_RUMBLE;
	play_event.id = m_vibration_event_uploaded ? m_vibration_event : -1;
	play_event.u.rumble.strong_magnitude = (unsigned short)(left * 65535);
	play_event.u.rumble.weak_magnitude = (unsigned short)(right * 65535);
	play_event.replay.length = play_event_time_length;
	play_event.replay.delay = 0;

	int retval = ioctl(m_file_handle, EVIOCSFF, &play_event);
	if (retval != -1) 
	{
		m_vibration_event = play_event.id;
	}
	else
	{
		DBG_LOG("EVIOCSFF Failed: %i", retval);
	}

	m_vibration_event_uploaded = true;
}

void Linux_JoystickState::StartVibrationEvent()
{
	struct input_event vib_event;

	vib_event.type = EV_FF;
	vib_event.code = m_vibration_event;
	vib_event.value = 1;

	write(m_file_handle, (const void*)&vib_event, sizeof(vib_event));

	m_last_vibration_start = Platform::Get()->Get_Ticks();
}

int Linux_JoystickState::CorrectAxis(int which, int value)
{
	/*
	JoystickProperties& axis_properties = m_caps.axis_properties[which];

	if (axis_properties.used) 
	{
		if (m_event.value > axis_properties.coef[0]) 
		{
			if (value < axis_properties.coef[1])
			{
				return 0;
			}
			value -= axis_properties.coef[1];
		} 
		else 
		{
			value -= axis_properties.coef[0];
		}
		value *= axis_properties.coef[2];
		value >>= 14;
	}
*/

	/* Clamp and return */
	if (value < -32768) return -32768;
	if (value >  32767) return  32767;

	return value;
}

void Linux_JoystickState::Update_State()
{
	// Open mount point.
	if (m_file_handle < 0)
	{
		if (Get_Controller_Mount_Point(m_mount_point, m_source_index))
		{
			m_file_handle = open(m_mount_point, O_RDWR|O_NONBLOCK);
			if (m_file_handle > 0)
			{
				ioctl(m_file_handle, EVIOCGNAME(256),	m_caps.name);
				ioctl(m_file_handle, JSIOCGAXES,		&m_caps.axes);
				ioctl(m_file_handle, JSIOCGBUTTONS,		&m_caps.buttons);	
				ioctl(m_file_handle, EVIOCGUNIQ(256),	m_caps.guid);

				m_is_vmware = (strstr(m_caps.name, "VMware") != NULL);				
				m_connected = true;
				m_event_bytes_read = 0;
				m_guid_product_str = "";

				m_caps.button_map_size = 0;
				m_caps.axis_map_size = 0;
				m_caps.hat_map_size = 0;

				struct input_id inpid;

			    if (ioctl(m_file_handle, EVIOCGID, &inpid) >= 0) 
			    {
					u8 guid[16];
				    memset(guid, 0, sizeof(guid));

				    u16* guid16 = (u16*)((char*)guid);
			
				    *(guid16++) = inpid.bustype;
			    	*(guid16++) = 0;

				    if (inpid.vendor && inpid.product && inpid.version) 
				    {
				        *(guid16++) = inpid.vendor;
				        *(guid16++) = 0;
				        *(guid16++) = inpid.product;
				        *(guid16++) = 0;
				        *(guid16++) = inpid.version;
				        *(guid16++) = 0;
				    } 
				    else 
				    {
				        strncpy((char*)guid16, m_caps.name, sizeof(guid) - 4);
				    }

					m_guid_product_str = StringHelper::Format(
						"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
						guid[0], guid[1], guid[2],  guid[3],  guid[4],  guid[5],  guid[6],  guid[7],
						guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]
					);
			    }
				
				unsigned long keybit[NBITS(KEY_MAX)] = { 0 };
				unsigned long absbit[NBITS(ABS_MAX)] = { 0 };
				unsigned long relbit[NBITS(REL_MAX)] = { 0 };

				/* See if this device uses the new unified event API */
				if ((ioctl(m_file_handle, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit) >= 0) &&
					(ioctl(m_file_handle, EVIOCGBIT(EV_ABS, sizeof(absbit)), absbit) >= 0) &&
					(ioctl(m_file_handle, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit) >= 0)) 
				{
					for (int i = BTN_JOYSTICK; i < KEY_MAX; ++i) 
					{
					    if (test_bit(i, keybit)) 
					    {
					        m_caps.button_map[i - BTN_MISC] = m_caps.button_map_size;
					        m_caps.button_map_size++;
					    }
					}
					for (int i = BTN_MISC; i < BTN_JOYSTICK; ++i) 
					{
					    if (test_bit(i, keybit)) 
					    {
					        m_caps.button_map[i - BTN_MISC] = m_caps.button_map_size;
					        m_caps.button_map_size++;
					    }
					}

				    for (int i = 0; i < ABS_MAX; ++i) 
				    {
			            /* Skip hats */
			            if (i == ABS_HAT0X) 
			            {
			                i = ABS_HAT3Y;
			                continue;
			            }

			            if (test_bit(i, absbit)) 
			            {
			                struct input_absinfo absinfo;

			                if (ioctl(m_file_handle, EVIOCGABS(i), &absinfo) < 0) 
			                {
			                    continue;
			                }

		                    m_caps.axis_map[i - BTN_MISC] = m_caps.axis_map_size;
					        m_caps.axis_map_size++;
						
			                m_caps.axis_properties[i].absinfo = absinfo;
/*
							printf("Joystick has absolute axis: 0x%.2x\n", i);
							printf("Values = { %d, %d, %d, %d, %d }\n",
								absinfo.value, absinfo.minimum, absinfo.maximum,
								absinfo.fuzz, absinfo.flat);
*/
			                if (absinfo.minimum == absinfo.maximum) 
			                {
			                    m_caps.axis_properties[i].used = 0;
			                } 
			                else 
			                {
			                    m_caps.axis_properties[i].used = 1;
			                    m_caps.axis_properties[i].coef[0] = (absinfo.maximum + absinfo.minimum) - 2 * absinfo.flat;
			                    m_caps.axis_properties[i].coef[1] = (absinfo.maximum + absinfo.minimum) + 2 * absinfo.flat;
			                    int t = ((absinfo.maximum - absinfo.minimum) - 4 * absinfo.flat);
			                    if (t != 0) 
			                    {
			                        m_caps.axis_properties[i].coef[2] = (1 << 28) / t;
			                    } 
			                    else 
			                    {
			                        m_caps.axis_properties[i].coef[2] = 0;
			                    }
			                }
			            }
			        }

					for (int i = ABS_HAT0X; i <= ABS_HAT3Y; i += 2) 
					{
						if (test_bit(i, absbit) || test_bit(i + 1, absbit)) 
						{
							int hat_index = (i - ABS_HAT0X) / 2;

			                struct input_absinfo& absinfo = m_caps.hat_properties[hat_index].absinfo;

							if (ioctl(m_file_handle, EVIOCGABS(i), &absinfo) < 0) 
							{
								continue;
							}

						    m_caps.hat_map[i - ABS_HAT0X] = m_caps.hat_map_size;
					        m_caps.hat_map_size++;
						    m_caps.hat_map[(i + 1) - ABS_HAT0X] = m_caps.hat_map_size;
					        m_caps.hat_map_size++;
						
/*
							printf("Joystick has hat %d\n", hat_index);
							printf("Values = { %d, %d, %d, %d, %d }\n",
								absinfo.value, absinfo.minimum, absinfo.maximum,
								absinfo.fuzz, absinfo.flat);
*/
						}
					}
			    }

				DBG_LOG("Joystick %i connected (name=%s axes=%u buttons=%u guid=%s mapping=%s).", 
					m_source_index, 
					m_caps.name, 
					m_caps.axes, 
					m_caps.buttons,
					m_guid_product_str.c_str(),
					Input::Get_Joystick_Mapping(m_guid_product_str.c_str())->Name.c_str()
				);
			}
		}
	}

	if (m_file_handle >= 0)
	{
		/*
		std::string fmt = "Axis: ";
		for (int i = 0; i < 7; i++)
		{
			fmt += StringHelper::Format("[%i]=%f ", i, (float)m_working_state.axis[i]);
		}
		printf("%s\n", fmt.c_str());
		*/
		/*
		std::string fmt = "Button: ";
		for (int i = 0; i < 7; i++)
		{
			fmt += StringHelper::Format("[%i]=%f ", i, (float)m_working_state.button[i]);
		}
		printf("%s\n", fmt.c_str());
		*/

	}

	// If open pump event queue.
	while (m_file_handle >= 0)
	{
		int bytes_remaining = sizeof(struct input_event) - m_event_bytes_read;

		int bytes = read(m_file_handle, ((char*)(&m_event)) + m_event_bytes_read, bytes_remaining);
		if (bytes > 0)
		{
			if (bytes < bytes_remaining)
			{
				m_event_bytes_read += bytes;
			}
			else
			{
				m_event_bytes_read = 0;

				if (m_event.type != EV_SYN && m_event.type != EV_ABS)
				{
				//	DBG_LOG("Event: type=%i code=%i value=%i", m_event.type, m_event.code, m_event.value);
				}

				switch (m_event.type)
				{
				case EV_ABS:
					{
						int axis_index = m_event.code;
						if (axis_index >= 0 && axis_index < JoystickDataState::max_axis)
						{
							if (axis_index >= ABS_HAT0X && axis_index <= ABS_HAT3Y)
							{
								axis_index = m_caps.hat_map[axis_index - ABS_HAT0X];
								m_working_state.hat_axis[axis_index] = m_event.value;
							}
							else
							{
								axis_index = m_caps.axis_map[axis_index];
								m_working_state.axis[m_event.code] = CorrectAxis(m_event.code, m_event.value);
							}
						}
						break;
					}

				case EV_KEY:
					{
						int button_index = m_event.code - BTN_MISC;

						if (button_index >= 0 && button_index < JoystickDataState::max_buttons)
						{
							button_index = m_caps.button_map[button_index];
							m_working_state.button[button_index] = m_event.value;
						}
						break;
					}
				}
			}
		}
		else if (bytes == 0)
		{
			DBG_LOG("Joystick %i disconnected.", m_source_index);

			m_connected = false;

			close(m_file_handle);
			m_file_handle = -1;
		}
		else
		{
			break;
		}
	}

	// Pump vibration events.
	if (m_file_handle >= 0)
	{
		/*
		if (m_source_index == 0)
		{
			if (m_test_dir == 0)
			{
				m_vibration_left += 0.001f;
				if (m_vibration_left >= 1.0f)
				{
					m_vibration_left = 1.0f;
					m_test_dir = 1;
				}
			}
			else
			{
				m_vibration_left -= 0.001f;
				if (m_vibration_left <= 0.0f)
				{
					m_vibration_left = 0.0f;
					m_test_dir = 0;
				}
			}

			DBG_LOG("Vib[%i]:%f %i", m_source_index, m_vibration_left, m_test_dir);
		}

		m_vibration_right = m_vibration_left;
		*/

		if (m_vibration_left != m_vibration_left_previous || m_vibration_right != m_vibration_right_previous)
		{
			if (!m_vibration_event_uploaded)
			{
				UploadVibrationEvent(m_vibration_left, m_vibration_right);
				StartVibrationEvent();
			}
			else
			{
				UploadVibrationEvent(m_vibration_left, m_vibration_right);

				double elapsed = (Platform::Get()->Get_Ticks() - m_last_vibration_start);
				if (elapsed > play_event_time_length)
				{
					StartVibrationEvent();
					m_last_vibration_start = Platform::Get()->Get_Ticks();
				}
			}
		}

		m_vibration_left_previous = m_vibration_left;
		m_vibration_right_previous = m_vibration_right;
	}
}

void Linux_JoystickState::Tick(const FrameTime& time)
{
	Update_State();

	if (m_time_since_last_input != FLT_MAX)
		m_time_since_last_input += time.Get_Frame_Time();

	if (m_connected == true)
	{
		m_has_existed = true;
		m_connection_frames++;

		Copy_State();

		bool bHasInput = false;

		for (int i = InputBindings::JOYSTICK_START + 1; i < InputBindings::JOYSTICK_END; i++)
		{
			InputBindings::Type key = (InputBindings::Type)i;

			if (Was_Key_Pressed(key))
			{
				bHasInput = true;
			}
		}

		if (bHasInput)
		{
			m_time_since_last_input = 0.0f;
		}
	}
	else
	{
		m_connection_frames = 0;
	}
}

bool Linux_JoystickState::Is_Connected()
{
	return m_connected;
}

bool Linux_JoystickState::Has_Input()
{
	for (int i = InputBindings::JOYSTICK_START + 1; i < InputBindings::JOYSTICK_END; i++)
	{
		InputBindings::Type key = (InputBindings::Type)i;

		if (Is_Key_Down(key))
		{
			return true;
		}
	}

	return false;
}

bool Linux_JoystickState::Get_Physical_Key_State(JoystickDataState& info, JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis)
{
	if (type == JoystickMappingInputType::Axis)
	{
		if (index >= 0 && index < (int)m_caps.max_axis)
		{
			if (!mapping->Axis_Properties[index].Is_Enabled)
			{
				return false;
			}

			float current = info.axis[index];
			float min_value = m_caps.axis_properties[index].absinfo.minimum;
			float max_value = m_caps.axis_properties[index].absinfo.maximum;
			
			float val = (current - min_value) * 2.0f / (max_value - min_value) - 1.0f;
			if (!half_axis)
			{
				val = (float)(current - min_value) / (float)(max_value - min_value);
			}
			
			if (inverted)
			{
				return val < 0 && fabs(val) > m_deadzone;
			}
			else
			{
				return val > 0 && fabs(val) > m_deadzone;
			}

			return false;
		}
	}
	else if (type == JoystickMappingInputType::Hat)
	{
		int hat_index = index / 4;
		int hat_direction = index %  4;

		if (hat_index >= 0 && hat_index < 4)
		{
			if (!mapping->Hat_Properties[index].Is_Enabled)
			{
				return false;
			}

			float x_value = info.hat_axis[(hat_index * 2)];
			float y_value = info.hat_axis[(hat_index * 2) + 1];

			     if (hat_direction == 0) return x_value > 0 && abs(x_value) > m_deadzone; // Right
			else if (hat_direction == 1) return x_value < 0 && abs(x_value) > m_deadzone; // Left
			else if (hat_direction == 2) return y_value > 0 && abs(y_value) > m_deadzone; // Up
			else if (hat_direction == 3) return y_value < 0 && abs(y_value) > m_deadzone; // Down

			return false;

/*
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
*/
		}
	}
	else if (type == JoystickMappingInputType::Button)
	{
		if (index >= 0 && index < (int)m_caps.max_buttons)
		{
			if (!mapping->Button_Properties[index].Is_Enabled)
			{
				return false;
			}
			return info.button[index] != 0;
		}
	}

	return false;
}

bool Linux_JoystickState::Get_State(JoystickDataState& info, InputBindings::Type type)
{
	GfxDisplay* display = GfxDisplay::Get();

	if (!Is_Connected())
	{
		return false;
	}
		
	// If display is not active, we don't want this input..
	if (display != NULL && !display->Is_Active())
	{
		return false;
	}

	// If this is a vmware mouse then return false.
	if (m_is_vmware == true)
	{
		return false;
	}

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
		return Get_Physical_Key_State(info, mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, axes_inverted, half_axis);
	}

	// POVS
	else if (type > InputBindings::JOYSTICK_HAT_START && type < InputBindings::JOYSTICK_HAT_END)
	{
		int hat_value_input = ((type - InputBindings::JOYSTICK_HAT_START) - 1);
		int hat_index = hat_value_input;

		float dpad = -1.0f;

		// Remap based on mappings.
		JoystickMappingProperties& mapping_props = mapping->Hat_Mappings[hat_index];
		return Get_Physical_Key_State(info, mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, false, false);
	}

	// Buttons
	else if (type >= InputBindings::Joystick_Button1 && type <= InputBindings::Joystick_Button32)
	{
		int button_id = ((type - InputBindings::JOYSTICK_BUTTON_START) - 1);

		// Remap based on mappings.
		JoystickMappingProperties& mapping_props = mapping->Button_Mappings[button_id];
		return Get_Physical_Key_State(info, mapping, mapping_props.Destination_Index, mapping_props.Destination_Type, false, false);
	}

	return false;
}

bool Linux_JoystickState::Is_Key_Down(InputBindings::Type type)
{
	return Get_State(m_current_state, type);
}

bool Linux_JoystickState::Was_Key_Down(InputBindings::Type type)
{
	return Get_State(m_previous_state, type);
}

bool Linux_JoystickState::Was_Key_Pressed(InputBindings::Type type)
{
	return Is_Key_Down(type) && !Was_Key_Down(type);
}

std::string Linux_JoystickState::Read_Character()
{
	// TODO: SteamController text input.
	return "";
}

void Linux_JoystickState::Flush_Character_Stack()
{
	// TODO: SteamController text input.
}

float Linux_JoystickState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

JoystickIconSet::Type Linux_JoystickState::Get_Icon_Set()
{
	return Input::Get_Joystick_Mapping(m_guid_product_str)->Icon_Set;
}

std::string Linux_JoystickState::Get_UniqueID()
{
	return StringHelper::Format("LinuxJoystick_%s", m_caps.guid);
}

Linux_JoystickState* g_joystick_states[MAX_INPUT_SOURCES];
bool g_joystick_states_created = false;

std::vector<JoystickState*> Linux_JoystickState::Get_All_Joysticks()
{
	if (!g_joystick_states_created)
	{
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			g_joystick_states[i] = new Linux_JoystickState(i);
		}

		g_joystick_states_created = true;
	}	

	std::vector<JoystickState*> states;

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		if (g_joystick_states[i] != NULL)
		{
			states.push_back(g_joystick_states[i]);
		}
	}

	return states;
}

std::vector<JoystickState*> Linux_JoystickState::Get_Known_Joysticks()
{
	std::vector<JoystickState*> states = Get_All_Joysticks();
	std::vector<JoystickState*> result;

	for (unsigned int i = 0; i < states.size(); i++)
	{
		if (static_cast<Linux_JoystickState*>(states[i])->m_has_existed)
		{
			result.push_back(states[i]);
		}
	}

	return result;
}

void Linux_JoystickState::Scan_For_Devices()
{
	// Nothing to do here.
}

#endif