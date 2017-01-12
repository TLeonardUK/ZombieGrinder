// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Engine/Input/Input.h"
#include "Engine/Input/MacOS/MacOS_JoystickState.h"
#include "Engine/Display/MacOS/MacOS_GfxDisplay.h"
#include "Engine/Platform/Platform.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/errno.h>
#include <sysexits.h>
#include <mach/mach.h>
#include <mach/mach_error.h>

#include <CoreFoundation/CoreFoundation.h>
#ifdef MACOS_FF_SUPPORT
#include <ForceFeedback/ForceFeedback.h>
#endif
#include <Carbon/Carbon.h>

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


// Bunch of this is based on code in blitzmax's freejoy module. Thx :3

static bool			g_hids_enumerated = false;
static mach_port_t	g_master_port;
static int			g_joystick_count = 0;
static JoystickCaps g_joystick_instances[MAX_INPUT_SOURCES];

void Open_Device(io_object_t device, IOHIDDeviceInterface**& outDevice, IOHIDDeviceRef& outDeviceRef)
{
	IOHIDDeviceInterface	**handle;
	IOCFPlugInInterface 	**plug;
	SInt32 					score;
	io_name_t				cls;
	IOReturn 				iores;
	HRESULT 				res;

	iores = IOObjectGetClass(device, cls);
	if (iores != kIOReturnSuccess) 
	{
		DBG_LOG("IOObjectGetClass failed with result 0x%08x.", iores);
		return;
	}

	iores = IOCreatePlugInInterfaceForService(device, kIOHIDDeviceUserClientTypeID, kIOCFPlugInInterfaceID, &plug, &score);
	if (iores != kIOReturnSuccess) 
	{
		DBG_LOG("IOCreatePlugInInterfaceForService failed with result 0x%08x.", iores);
		return;
	}

	res = (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOHIDDeviceInterfaceID), (void**)&handle);
	if (res != S_OK)
	{
		DBG_LOG("QueryInterface failed with result 0x%08x.", res);
		return;
	}

	res = (*handle)->open(handle, 0);
	if (res != S_OK)
	{
		DBG_LOG("Open HID failed with result 0x%08x.", res);
		handle = NULL;
	}

	(*plug)->Release(plug);

	outDevice = handle;
	outDeviceRef = IOHIDDeviceCreate(kCFAllocatorDefault, (io_service_t)device);
}

void Extract_Joystick_Caps_Element(CFTypeRef element, JoystickCaps& joystick)
{
	long number;
	CFTypeRef object;

	// Get HID uniquely identifying cookie.
	object = CFDictionaryGetValue((CFDictionaryRef)element, CFSTR(kIOHIDElementCookieKey));
	if (object == NULL || 
		CFGetTypeID(object) != CFNumberGetTypeID() ||
		!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))  
	{
		return;
	}	
	IOHIDElementCookie cookie = (IOHIDElementCookie)number;

	// Get what data this element contains.
	object = CFDictionaryGetValue((CFDictionaryRef)element, CFSTR(kIOHIDElementUsageKey));
	if (object == NULL || 
		CFGetTypeID(object) != CFNumberGetTypeID() ||
		!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number)) 
	{
		return;
	}
	long usage = number;

	// Grab axis counts!
	long axismax = 0;
	long axismin = 0;
	object = CFDictionaryGetValue((CFDictionaryRef)element, CFSTR(kIOHIDElementMaxKey));
	if (object != NULL && CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))
	{
		axismax = number;
	}

	object = CFDictionaryGetValue((CFDictionaryRef)element, CFSTR(kIOHIDElementMinKey));
	if (object != NULL && CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))
	{
		axismin = number;
	}

	// Grab the page the HID is on (why the fuck is it called a page!?).
	object = CFDictionaryGetValue((CFDictionaryRef)element, CFSTR(kIOHIDElementUsagePageKey));
	if (object == NULL || 
		CFGetTypeID(object) != CFNumberGetTypeID() ||
		!CFNumberGetValue((CFNumberRef)object, kCFNumberLongType, &number))
	{
		return;
	}
	long page = number;

	switch (page)
	{
	case kHIDPage_GenericDesktop:
		{
			switch (usage)
			{
			case kHIDUsage_GD_X:		
			case kHIDUsage_GD_Y:		
			case kHIDUsage_GD_Z:		
			case kHIDUsage_GD_Rz:		
			case kHIDUsage_GD_Ry:		
			case kHIDUsage_GD_Rx:			
			case kHIDUsage_GD_Slider:
			case kHIDUsage_GD_Dial:	
			case kHIDUsage_GD_Wheel:	
				{			
					if (std::find(joystick.element_cookies.begin(), joystick.element_cookies.end(), cookie) == joystick.element_cookies.end())
					{
						joystick.axis_properties[joystick.axis_count].cookie = cookie;
						joystick.axis_properties[joystick.axis_count].minimum = axismin;
						joystick.axis_properties[joystick.axis_count].maximum = axismax;
						joystick.axis_properties[joystick.axis_count].sort_value = usage;
						joystick.axis_count++;
						joystick.element_cookies.push_back(cookie);
					}
					break;
				}

			case kHIDUsage_GD_Hatswitch:
				{
					if (std::find(joystick.element_cookies.begin(), joystick.element_cookies.end(), cookie) == joystick.element_cookies.end())
					{
						joystick.hat_properties[joystick.hat_count].cookie = cookie;
						joystick.hat_properties[joystick.hat_count].minimum = axismin;
						joystick.hat_properties[joystick.hat_count].maximum = axismax;
						joystick.hat_properties[joystick.hat_count].sort_value = usage;
						joystick.hat_count++;
						joystick.element_cookies.push_back(cookie);
					}
					break;
				}

			case kHIDUsage_GD_DPadUp:
			case kHIDUsage_GD_DPadDown:
			case kHIDUsage_GD_DPadLeft:
			case kHIDUsage_GD_DPadRight:
				{
					if (std::find(joystick.element_cookies.begin(), joystick.element_cookies.end(), cookie) == joystick.element_cookies.end())
					{
						joystick.button_properties[joystick.button_count].cookie = cookie;
						joystick.button_properties[joystick.button_count].sort_value = usage;
						joystick.button_count++;
						joystick.element_cookies.push_back(cookie);
					}
					break;
				}
			}
			break;
		}

	case kHIDPage_Simulation:
		{
			switch (usage)
			{
				case kHIDUsage_Sim_Throttle:
				case kHIDUsage_Sim_Rudder:
				{					
					if (std::find(joystick.element_cookies.begin(), joystick.element_cookies.end(), cookie) == joystick.element_cookies.end())
					{
						joystick.axis_properties[joystick.axis_count].cookie = cookie;
						joystick.axis_properties[joystick.axis_count].minimum = axismin;
						joystick.axis_properties[joystick.axis_count].maximum = axismax;
						joystick.hat_properties[joystick.hat_count].sort_value = usage;
						joystick.axis_count++;
						joystick.element_cookies.push_back(cookie);
					}
					break;
				}
			}			
		}

	case kHIDPage_Button:
	case kHIDPage_Consumer:
		{
			if (std::find(joystick.element_cookies.begin(), joystick.element_cookies.end(), cookie) == joystick.element_cookies.end())
			{
				joystick.button_properties[joystick.button_count].cookie = cookie;
				joystick.button_properties[joystick.button_count].sort_value = usage;
				joystick.button_count++;
				joystick.element_cookies.push_back(cookie);
			}
			break;
		}
	}
}

void Extract_Joystick_Caps(CFTypeRef properties, JoystickCaps& joystick)
{
	if (properties == NULL)
	{
		return;
	}

	CFTypeID type = CFGetTypeID(properties);

	if (type == CFArrayGetTypeID())
	{
		int count = CFArrayGetCount((CFArrayRef)properties);
		for (int i = 0; i < count; i++)
		{
			const void* obj = CFArrayGetValueAtIndex((CFArrayRef)properties, i);
			Extract_Joystick_Caps(obj, joystick);
		}

		return;
	}

	if (type == CFDictionaryGetTypeID())
	{
		Extract_Joystick_Caps_Element(properties, joystick);

		int				count	= CFDictionaryGetCount((CFDictionaryRef)properties);
		const void **	keys	= (const void**)malloc(count * sizeof(void*));
		const void **	vals	= (const void**)malloc(count * sizeof(void*));

		CFDictionaryGetKeysAndValues((CFDictionaryRef)properties, keys, vals);

		for (int i = 0; i < count; i++)
		{
			CFTypeRef key = keys[i];
			CFTypeRef val = vals[i];

			type = CFGetTypeID(key);

			if (type == CFStringGetTypeID())
			{
				const char* value = CFStringGetCStringPtr((CFStringRef)key, CFStringGetSystemEncoding());
				if (value != NULL)
				{
					Extract_Joystick_Caps(val, joystick);
				}
			}
		}

		free(vals);
		free(keys);
		return;
	}
}

#ifdef MACOS_FF_SUPPORT
const char* FFERRToString(int err)
{
	switch (err)
	{
#define ERRCODE(err) case err: return #err;
		ERRCODE(FFEFF_OBJECTOFFSETS)
		ERRCODE(FFERR_DEVICEFULL)
		ERRCODE(FFERR_DEVICEPAUSED)
		ERRCODE(FFERR_DEVICERELEASED)
		ERRCODE(FFERR_EFFECTPLAYING)
		ERRCODE(FFERR_EFFECTTYPEMISMATCH)
		ERRCODE(FFERR_EFFECTTYPENOTSUPPORTED)
		ERRCODE(FFERR_GENERIC)
		ERRCODE(FFERR_HASEFFECTS)
		ERRCODE(FFERR_INCOMPLETEEFFECT)
		ERRCODE(FFERR_INTERNAL)
		ERRCODE(FFERR_INVALIDDOWNLOADID)
		ERRCODE(FFERR_INVALIDPARAM)
		ERRCODE(FFERR_MOREDATA)
		ERRCODE(FFERR_NOINTERFACE)
		ERRCODE(FFERR_NOTDOWNLOADED)
		ERRCODE(FFERR_NOTINITIALIZED)
		ERRCODE(FFERR_OUTOFMEMORY)
		ERRCODE(FFERR_UNPLUGGED)
		ERRCODE(FFERR_UNSUPPORTED)
		ERRCODE(FFERR_UNSUPPORTEDAXIS)
#undef ERRCODE
	}

	return "Unknown Error";
}
#endif

bool SortJoystickProperties(const JoystickProperties& a, const JoystickProperties& b)
{
	return a.sort_value < b.sort_value;
}

void Enumerate_HIDs(UInt32 page, UInt32 usage)
{
	CFTypeRef 				element;	
	IOHIDDeviceInterface**	device;
	struct macjoy*			joy;

	CFMutableDictionaryRef dic = IOServiceMatching(kIOHIDDeviceKey);
	if (dic == NULL)
	{
		DBG_LOG("No directory returned from IOServiceMatching.");
		return;
	}

	CFNumberRef rpage  = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
	CFNumberRef rusage = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);

	CFDictionarySetValue(dic, CFSTR(kIOHIDPrimaryUsagePageKey), rpage);
	CFDictionarySetValue(dic, CFSTR(kIOHIDPrimaryUsageKey), rusage);

	io_iterator_t iter;
	IOReturn res = IOServiceGetMatchingServices(g_master_port, dic, &iter);
	if (res != kIOReturnSuccess)
	{
		DBG_LOG("IOServiceGetMatchingServices failed with error 0x%08x", res);
		return;
	}
	
	io_object_t obj;
	while ((obj = IOIteratorNext(iter)) &&
		   g_joystick_count < MAX_INPUT_SOURCES)
	{
		JoystickCaps caps;
		memset(&caps, 0, sizeof(JoystickCaps));

		Open_Device(obj, caps.device, caps.device_ref);
#ifdef MACOS_FF_SUPPORT
		caps.ffdevice = NULL;
#endif

		if (caps.device != NULL)
		{
			CFMutableDictionaryRef properties;
			res = IORegistryEntryCreateCFProperties(obj, &properties, kCFAllocatorDefault, kNilOptions);
			if (res != kIOReturnSuccess)
			{
				DBG_LOG("IORegistryEntryCreateCFProperties failed with error 0x%08x", res);
				continue;
			}

			long number;
			CFTypeRef object;
			
			// Generate a GUID.
			char product[128];
			object = IOHIDDeviceGetProperty(caps.device_ref, CFSTR(kIOHIDProductKey));
			if (!object)
			{
				object = IOHIDDeviceGetProperty(caps.device_ref, CFSTR(kIOHIDManufacturerKey));				
			}
			if ((!object) || (!CFStringGetCString((CFStringRef)object, product, sizeof(product), kCFStringEncodingUTF8)))
			{
				strncpy(product, "Unidentified joystick", sizeof(product) - 1);
			}

			u8 guid[16];
			memset(guid, 0, sizeof(guid));

			object = IOHIDDeviceGetProperty(caps.device_ref, CFSTR(kIOHIDVendorIDKey));
			if (object)
			{
				CFNumberGetValue((CFNumberRef)object, kCFNumberSInt32Type, &guid[0]);
			}

			object = IOHIDDeviceGetProperty(caps.device_ref, CFSTR(kIOHIDProductIDKey));
			if (object)
			{
				CFNumberGetValue((CFNumberRef)object, kCFNumberSInt32Type, &guid[8]);
			}
			
			object = IOHIDDeviceGetProperty(caps.device_ref, CFSTR(kIOHIDSerialNumberKey));
			if (object)
			{
				CFStringGetCString((CFStringRef)object, caps.guid, sizeof(caps.guid), kCFStringEncodingUTF8);
			}

			u32* guid32 = (u32*)guid;
			if (!guid32[0] && !guid32[1])
			{
				const u16 BUS_BLUETOOTH = 0x05;
				u16* guid16 = (u16*)guid32;
				*guid16++ = BUS_BLUETOOTH;
				*guid16++ = 0;
				strncpy((char*)guid16, product, sizeof(guid) - 4);
			}

			caps.guid_product_str = StringHelper::Format(
				"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				guid[0], guid[1], guid[2],  guid[3],  guid[4],  guid[5],  guid[6],  guid[7],
				guid[8], guid[9], guid[10], guid[11], guid[12], guid[13], guid[14], guid[15]
			);

			DBG_LOG("HID device found with GUID %s (Mapping=%s)", 
				caps.guid_product_str.c_str(),
				Input::Get_Joystick_Mapping(caps.guid_product_str.c_str())->Name.c_str()
			);

			DBG_LOG("Extracting joystick capabilities for HID %i.", g_joystick_count);

			Extract_Joystick_Caps(properties, caps);

			// Sort axis etc by usage.
			std::sort(caps.button_properties, caps.button_properties + caps.button_count, &SortJoystickProperties);
			std::sort(caps.axis_properties, caps.axis_properties + caps.axis_count, &SortJoystickProperties);
			std::sort(caps.hat_properties, caps.hat_properties + caps.hat_count, &SortJoystickProperties);

			DBG_LOG("Successfully parsed HID Device (%i buttons, %i axis, %i hats).", caps.button_count, caps.axis_count, caps.hat_count);
			
#ifdef MACOS_FF_SUPPORT
			caps.ffenabled = false;
#endif

			g_joystick_instances[g_joystick_count++] = caps;

#ifdef MACOS_FF_SUPPORT
			if (FFIsForceFeedback(obj) == FF_OK)
			{
				DBG_LOG("Creating force feedback device.");

				res = FFCreateDevice(obj, &caps.ffdevice);
				if (res != FF_OK)
				{
					DBG_LOG("Open force feedback device failed with result 0x%08x.", res);
				}
				else
				{
					FFDeviceGetForceFeedbackCapabilities(caps.ffdevice, &caps.ffcaps);
					DBG_LOG("  supportedEffects = 0x%08x", caps.ffcaps.supportedEffects);
					DBG_LOG("  emulatedEffects = 0x%08x", caps.ffcaps.emulatedEffects);
					DBG_LOG("  axesCount = %i", caps.ffcaps.numFfAxes);

					for (int i = 0; i < caps.ffcaps.numFfAxes; i++)
					{
						DBG_LOG("  axesCount[%i] = %i", i,  caps.ffcaps.ffAxes[i]);
					}

					if (caps.ffcaps.numFfAxes > 0 && (caps.ffcaps.supportedEffects & FFCAP_ET_SINE) != 0)
					{
						FFDeviceSendForceFeedbackCommand(caps.ffdevice, FFSFFC_RESET);
						FFDeviceSendForceFeedbackCommand(caps.ffdevice, FFSFFC_SETACTUATORSON);
					
						caps.ffcustomparamsaxis[0] = caps.ffcaps.ffAxes[0];

						caps.ffcustomparamsdirection[0] = 10000;
						caps.ffcustomparamsdirection[1] = 0;

						memset(&caps.ffcustomparams, 0, sizeof(FFPERIODIC));
						caps.ffcustomparams.dwMagnitude = FF_FFNOMINALMAX;
						caps.ffcustomparams.lOffset = 0;
						caps.ffcustomparams.dwPhase = 0;
						caps.ffcustomparams.dwPeriod = 100 * 1000;

						memset(&caps.ffparams, 0, sizeof(FFEFFECT));
						caps.ffparams.dwSize = sizeof(FFEFFECT);
						caps.ffparams.dwFlags = FFEFF_CARTESIAN | FFEFF_OBJECTOFFSETS;
						caps.ffparams.dwDuration = FF_INFINITE;
						caps.ffparams.dwSamplePeriod = 0;
						caps.ffparams.dwGain = FF_FFNOMINALMAX;
						caps.ffparams.dwTriggerButton = FFEB_NOTRIGGER;
						caps.ffparams.dwTriggerRepeatInterval = 0;
						caps.ffparams.cAxes = 1;
						caps.ffparams.rgdwAxes = caps.ffcustomparamsaxis;
						caps.ffparams.rglDirection = caps.ffcustomparamsdirection;
						caps.ffparams.lpEnvelope = 0;
						caps.ffparams.cbTypeSpecificParams = sizeof(FFPERIODIC);
						caps.ffparams.lpvTypeSpecificParams = &caps.ffcustomparams;
						caps.ffparams.dwStartDelay = 0;
					
						res = FFDeviceCreateEffect(caps.ffdevice, kFFEffectType_Sine_ID, &caps.ffparams, &caps.ffeffect);
						if (res != FF_OK)
						{
							DBG_LOG("Create force feedback effect failed with result 0x%08x (%s).", res, FFERRToString(res));	
						}
						else
						{
							caps.ffenabled = true;
						}
					}
				}
			}
			else
			{
				DBG_LOG("HID does not support force feedback.");
			}
#endif
		}
		else
		{
			DBG_LOG("Failed to open HID Device.");
		}
	}

	IOObjectRelease(iter);
}

MacOS_JoystickState::MacOS_JoystickState(int source_index)
	: m_source_index(source_index)
	, m_deadzone(0.5f)
	, m_time_since_last_input(FLT_MAX)
	, m_connection_frames(0)
	, m_running(true)
	, m_connected(false)
#ifdef MACOS_FF_SUPPORT
	, m_vibration_left(0.0f)
	, m_vibration_right(0.0f)
	, m_vibration_left_previous(0.0f)
	, m_vibration_right_previous(0.0f)
	, m_last_vibration_start(0.0f)
	, m_test_dir(0)
#endif
{
	memset(&m_current_state, 0, sizeof(JoystickDataState));
	memset(&m_previous_state, 0, sizeof(JoystickDataState));

	if (g_hids_enumerated == false)
	{
		memset(g_joystick_instances, 0, sizeof(g_joystick_instances));

		DBG_LOG("Enumerating joystick HID's.");
		int res = IOMasterPort(bootstrap_port, &g_master_port);
		if (res == 0)
		{
			Enumerate_HIDs(kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
			Enumerate_HIDs(kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);
			Enumerate_HIDs(kHIDPage_GenericDesktop, kHIDUsage_GD_MultiAxisController);
		}
		else
		{
			DBG_LOG("IOMasterPort failed with error 0x%08x", res);
		}

		g_hids_enumerated = true;

		DBG_LOG("%i joystick HID's recognised.", g_joystick_count);
	}

	m_caps = g_joystick_instances[source_index];
}

void MacOS_JoystickState::Set_Vibration(float leftMotor, float rightMotor)
{
#ifdef MACOS_FF_SUPPORT
	m_vibration_left = leftMotor;
	m_vibration_right = rightMotor;
#endif
}

#ifdef MACOS_FF_SUPPORT
void MacOS_JoystickState::UploadVibrationEvent(float left, float right)
{
	m_caps.ffcustomparams.dwMagnitude = (int)(10000 * ((left+right) * 0.5f));

	int res = FFEffectSetParameters(m_caps.ffeffect, &m_caps.ffparams, FFEP_AXES|FFEP_DIRECTION|FFEP_TYPESPECIFICPARAMS);
	if (res != FF_OK)
	{
		DBG_LOG("FFEffectSetParameters failed with result 0x%08x.", res);										
	}

	m_vibration_event_uploaded = true;
}

void MacOS_JoystickState::StartVibrationEvent()
{
	int res = FFEffectStart(m_caps.ffeffect, FF_INFINITE, 0);
	if (res != FF_OK)
	{
		DBG_LOG("FFEffectSetParams failed with result 0x%08x.", res);										
	}
}
#endif

void MacOS_JoystickState::Tick(const FrameTime& time)
{
	m_connected = (g_joystick_count > m_source_index);

	if (m_connected == true)
	{
		m_has_existed = true;
		m_connection_frames++;

		memcpy(&m_previous_state, &m_current_state, sizeof(JoystickDataState));
		m_current_state = Read_State();

		if (m_time_since_last_input != FLT_MAX)
			m_time_since_last_input += time.Get_Frame_Time();

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

#ifdef MACOS_FF_SUPPORT
		// Update vibration.
		if (m_caps.ffenabled)
		{
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

				m_vibration_right = m_vibration_left;
			}

			if (m_vibration_left != m_vibration_left_previous || m_vibration_right != m_vibration_right_previous)
			{
				bool bNeedsStart = !m_vibration_event_uploaded;

				UploadVibrationEvent(m_vibration_left, m_vibration_right);

				if (bNeedsStart)
				{
					StartVibrationEvent();
				}
			}
		}

		m_vibration_left_previous = m_vibration_left;
		m_vibration_right_previous = m_vibration_right;
#endif
	}
	else
	{
		m_connection_frames = 0;
	}
}

bool MacOS_JoystickState::Is_Connected()
{
	return m_connected;
}

bool MacOS_JoystickState::Has_Input()
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

JoystickDataState MacOS_JoystickState::Read_State()
{
	JoystickDataState state;
	memset(&state, 0, sizeof(JoystickDataState));
	
	JoystickCaps& caps = g_joystick_instances[m_source_index];
	
    IOHIDEventStruct event;
	IOHIDElementCookie cookie;

	for (int i = 0; i < JoystickDataState::max_buttons; i++)
	{
		state.button[i] = false;

		if (i < caps.button_count)
		{
			cookie = caps.button_properties[i].cookie;
			if (cookie != NULL)
			{
				(*caps.device)->getElementValue(caps.device, cookie, &event);
				
				if (event.value)
				{
					state.button[i] = true;
				}
			}
		}
	}

	for (int i = 0; i < JoystickDataState::max_axis; i++)
	{
		state.axis[i] = 0.0f;
		
		if (i < caps.axis_count)
		{
			cookie = caps.axis_properties[i].cookie;
			if (cookie != NULL)
			{
				(*caps.device)->getElementValue(caps.device, cookie, &event);				
				state.axis[i] = event.value;				
			}
		}
	}

	for (int i = 0; i < JoystickDataState::max_hats; i++)
	{
		state.hat[i] = 0.0f;
		
		if (i < caps.hat_count)
		{
			cookie = caps.hat_properties[i].cookie;
			if (cookie != NULL)
			{
				(*caps.device)->getElementValue(caps.device, cookie, &event);				
				state.hat[i] = event.value;				
			}
		}
	}

	return state;
}

bool MacOS_JoystickState::Has_Axis(int axis)
{
	return axis < g_joystick_instances[m_source_index].axis_count;
}

bool MacOS_JoystickState::Get_Physical_Key_State(JoystickDataState& info, JoystickMapping* mapping, int index, JoystickMappingInputType::Type type, bool inverted, bool half_axis)
{
	if (type == JoystickMappingInputType::Axis)
	{
		if (index >= 0 && index < (int)m_caps.axis_count)
		{
			if (!mapping->Axis_Properties[index].Is_Enabled)
			{
				return false;
			}

			float current = info.axis[index];
			float min_value = m_caps.axis_properties[index].minimum;
			float max_value = m_caps.axis_properties[index].maximum;
			
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

		if (hat_index >= 0 && hat_index < 4 && hat_index < m_caps.hat_count)
		{
			if (!mapping->Hat_Properties[index].Is_Enabled)
			{
				return false;
			}

			long value = (long)info.hat[hat_index];
			long minimum = m_caps.hat_properties[hat_index].minimum;
			long maximum = m_caps.hat_properties[hat_index].maximum;
			long range = maximum - minimum + 1;

			if (range == 4)
			{
				value *= 2;
			}
			else if (range != 8)
			{
				value = -1;
			}

			float x_value = 0.0f;
			float y_value = 0.0f;

			switch (value)
			{
				case 0:  x_value =  0.0f; y_value = -1.0f; break;
				case 1:  x_value =  1.0f; y_value = -1.0f; break;
				case 2:  x_value =  1.0f; y_value =  0.0f; break;
				case 3:  x_value =  1.0f; y_value =  1.0f; break;
				case 4:  x_value =  0.0f; y_value =  1.0f; break;
				case 5:  x_value = -1.0f; y_value =  1.0f; break;
				case 6:  x_value = -1.0f; y_value =  0.0f; break;
				case 7:	 x_value = -1.0f; y_value = -1.0f; break;
				default: x_value =  0.0f; y_value =  0.0f; break;				
			}

			     if (hat_direction == 0) return x_value > 0 && abs(x_value) > m_deadzone; // Right
			else if (hat_direction == 1) return x_value < 0 && abs(x_value) > m_deadzone; // Left
			else if (hat_direction == 2) return y_value > 0 && abs(y_value) > m_deadzone; // Up
			else if (hat_direction == 3) return y_value < 0 && abs(y_value) > m_deadzone; // Down

			return false;
		}
	}
	else if (type == JoystickMappingInputType::Button)
	{
		if (index >= 0 && index < (int)m_caps.button_count)
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

bool MacOS_JoystickState::Get_State(JoystickDataState& info, InputBindings::Type type)
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

	JoystickMapping* mapping = Input::Get_Joystick_Mapping(m_caps.guid_product_str);

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

bool MacOS_JoystickState::Is_Key_Down(InputBindings::Type type)
{
	return Get_State(m_current_state, type);
}

bool MacOS_JoystickState::Was_Key_Down(InputBindings::Type type)
{
	return Get_State(m_previous_state, type);
}

bool MacOS_JoystickState::Was_Key_Pressed(InputBindings::Type type)
{
	return Is_Key_Down(type) && !Was_Key_Down(type);
}

std::string MacOS_JoystickState::Read_Character()
{
	// TODO: SteamController text input.
	return "";
}

void MacOS_JoystickState::Flush_Character_Stack()
{
	// TODO: SteamController text input.
}

float MacOS_JoystickState::Time_Since_Last_Input()
{
	return m_time_since_last_input;
}

JoystickIconSet::Type MacOS_JoystickState::Get_Icon_Set()
{
	return Input::Get_Joystick_Mapping(m_caps.guid_product_str)->Icon_Set;
}

std::string MacOS_JoystickState::Get_UniqueID()
{
	return StringHelper::Format("MacOSJoystick_%s", m_caps.guid);
}

MacOS_JoystickState* g_joystick_states[MAX_INPUT_SOURCES];
bool g_joystick_states_created = false;


std::vector<JoystickState*> MacOS_JoystickState::Get_All_Joysticks()
{
	if (!g_joystick_states_created)
	{
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			g_joystick_states[i] = new MacOS_JoystickState(i);
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

std::vector<JoystickState*> MacOS_JoystickState::Get_Known_Joysticks()
{
	std::vector<JoystickState*> states = Get_All_Joysticks();
	std::vector<JoystickState*> result;

	for (unsigned int i = 0; i < states.size(); i++)
	{
		if (static_cast<MacOS_JoystickState*>(states[i])->m_has_existed)
		{
			result.push_back(states[i]);
		}
	}

	return result;
}

void MacOS_JoystickState::Scan_For_Devices()
{
	// Nothing to do here.
}

#endif