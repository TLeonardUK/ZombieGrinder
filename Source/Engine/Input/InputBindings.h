// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_INPUT_INPUTBINDINGS_
#define _ENGINE_INPUT_INPUTBINDINGS_

#include <vector>
#include <cstring>
#include <cstdlib>

//#include "Engine/Input/JoystickState.h"
#include "Engine/Localise/Locale.h"
#include "Engine/Resources/ResourceFactory.h"

struct InputBindingType
{
	enum Type
	{
		Any,
		Joystick,
		Keyboard
	};

	static const char* To_String (Type type)
	{
		static const char* g_names[] = {
			"Any",
			"Joystick",
			"Keyboard"
		};

		return g_names[(int)type];
	}
};

struct JoystickIconSet
{
	enum Type
	{
		Xbox360,
		XboxOne,
		DS3,
		DS4,
		Steam,

		COUNT
	};

	static Type Parse(const char* value, Type default_value)
	{
		if (stricmp(value, "Xbox360") == 0)
			return Xbox360;
		if (stricmp(value, "XboxOne") == 0)
			return XboxOne;
		if (stricmp(value, "DS3") == 0)
			return DS3;
		if (stricmp(value, "DS4") == 0)
			return DS4;
		if (stricmp(value, "Steam") == 0)
			return Steam;

		return default_value;
	}

	static const char* To_String(Type type)
	{
		static const char* g_names[] = {
			"Xbox360",
			"XboxOne",
			"DS3",
			"DS4",
			"Steam"
		};

		return g_names[(int)type];
	}

	static std::string GetInputDescription(std::string base_name, JoystickIconSet::Type set);
	static std::string GetAtlasFrameName(std::string base_atlas_frame, JoystickIconSet::Type set);
};

struct InputBindings
{
	enum Type
	{
#define BINDING(x, atlas_image) x,
#include "Engine/Input/InputBindings.inc"
#undef BINDING
	};

	static bool Parse(const char* name, InputBindings::Type& result)
	{
#define BINDING(x, atlas_image) if (stricmp(name, #x) == 0) { result = x; return true; }
#include "Engine/Input/InputBindings.inc"
#undef BINDING
		return false;
	}

	static bool To_String(InputBindings::Type& type, const char*& result)
	{
#define BINDING(x, atlas_image) if (x == type) { result = #x; return true; }
#include "Engine/Input/InputBindings.inc"
#undef BINDING
		return false;
	}

	static bool ToDescription(InputBindings::Type& type, std::string& result, JoystickIconSet::Type icon_set);
	static bool ToAtlasFrameName(InputBindings::Type& type, std::string& result, JoystickIconSet::Type icon_set);
};

#endif