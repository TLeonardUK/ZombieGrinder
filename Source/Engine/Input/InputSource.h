// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_SOURCE_
#define _ENGINE_INPUT_SOURCE_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Vector2.h"
#include "Engine/Engine/FrameTime.h"

#include "Engine/Input/MouseState.h"
#include "Engine/Input/KeyboardState.h"
#include "Engine/Input/JoystickState.h"

#include "Engine/Input/OutputBindings.h"
#include "Engine/Input/InputBindings.h"

#include <vector>

struct InputSourceDevice
{
	enum Type
	{
		NONE,
		Joystick,
		Keyboard
	};
};

struct InputSource
{
	MEMORY_ALLOCATOR(InputSource, "InputSpurce");

public:
	InputSourceDevice::Type Device;
	int						Device_Index;
 
public:	
	InputSource(InputSourceDevice::Type device, int index)
		: Device(device)
		, Device_Index(index)
	{
	}

	bool operator==(const InputSource& other)
	{
		return other.Device == Device && other.Device_Index == Device_Index;
	}

	bool operator!=(const InputSource& other)
	{
		return !operator==(other);
	}

};

#endif

