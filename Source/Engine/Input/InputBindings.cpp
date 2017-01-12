// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/InputBindings.h"

std::string JoystickIconSet::GetInputDescription(std::string base_name, JoystickIconSet::Type set)
{
	std::string override_string_name = "#" + base_name + "_" + JoystickIconSet::To_String(set);
	if (Locale::Get()->Get_String(override_string_name, true) != NULL)
	{
		return override_string_name;
	}
	return "#" + base_name;
}

std::string JoystickIconSet::GetAtlasFrameName(std::string base_atlas_frame, JoystickIconSet::Type set)
{
	std::string override_atlas_name = base_atlas_frame + "_" + JoystickIconSet::To_String(set);
	if (ResourceFactory::Get()->Get_Atlas_Frame(override_atlas_name.c_str()) != NULL)
	{
		return override_atlas_name;
	}
	return base_atlas_frame;
}

bool InputBindings::ToDescription(InputBindings::Type& type, std::string& result, JoystickIconSet::Type icon_set)
{
#define BINDING(x, atlas_image) if (x == type) { result = JoystickIconSet::GetInputDescription(#x, icon_set); return true; }
#include "Engine/Input/InputBindings.inc"
#undef BINDING
	return false;
}

bool InputBindings::ToAtlasFrameName(InputBindings::Type& type, std::string& result, JoystickIconSet::Type icon_set)
{
#define BINDING(x, atlas_image) if (x == type) { result = JoystickIconSet::GetAtlasFrameName(atlas_image, icon_set); return true; }
#include "Engine/Input/InputBindings.inc"
#undef BINDING
	return false;
}