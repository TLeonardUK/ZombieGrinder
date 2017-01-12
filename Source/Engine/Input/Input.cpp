// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Input/Input.h"

#include "Engine/Display/GfxDisplay.h"

#include "Engine/Input/Null/Null_Input.h"

#if defined(PLATFORM_WIN32)
#include "Engine/Input/Win32/Win32_Input.h"
#elif defined(PLATFORM_LINUX)
#include "Engine/Input/Linux/Linux_Input.h"
#elif defined(PLATFORM_MACOS)
#include "Engine/Input/MacOS/MacOS_Input.h"
#endif

#include "Engine/Input/Steam/Steam_Input.h"

#include "Engine/Online/OnlinePlatform.h"

#include "Engine/Engine/EngineOptions.h"

#include "Engine/IO/StreamFactory.h"
#include "Engine/Resources/ResourceFactory.h"
#include "Engine/Config/ConfigFile.h"

#include <stdint.h>
#include <stdlib.h>
#include <float.h>

#include "public/steam/steam_api.h"

//#define OPT_LOG_MAPPINGS

Input*							Input::g_input_sources[MAX_INPUT_SOURCES];
int								Input::m_input_source_count = 0;
std::vector<JoystickMapping>	Input::g_input_mappings;

#ifdef STEAM_CONTROLLER_SUPPORT
Input*							Input::g_steam_input = NULL;
#endif

JoystickState*					Input::g_joystick_slots[MAX_INPUT_SOURCES];
JoystickState*					Input::g_dummy_joystick_state = NULL;

bool							Input::g_mark_for_device_scan = false;

void Find_Next_Unbound_Mapping(
	bool* array_1, JoystickMappingInputType::Type array_1_type, int array_1_len,
	bool* array_2, JoystickMappingInputType::Type array_2_type, int array_2_len,
	bool* array_3, JoystickMappingInputType::Type array_3_type, int array_3_len,
	int& output_index,
	JoystickMappingInputType::Type& output_type)
{
	for (int i = 0; i < array_1_len; i++)
	{
		if (array_1[i] == false)
		{
			output_index = i;
			output_type = array_1_type;
			array_1[i] = true;
			return;
		}
	}

	for (int i = 0; i < array_2_len; i++)
	{
		if (array_2[i] == false)
		{
			output_index = i;
			output_type = array_2_type;
			array_2[i] = true;
			return;
		}
	}

	for (int i = 0; i < array_3_len; i++)
	{
		if (array_3[i] == false)
		{
			output_index = i;
			output_type = array_3_type;
			array_3[i] = true;
			return;
		}
	}

	output_index = -1;
}

void Input::Load_Joystick_Mapping(const char* url)
{
	g_input_mappings.clear();

	// Load configured controller mapping.
	ConfigFile file;
	if (file.Load(url))
	{
		std::vector<ConfigFileNode> bindings = file.Get_Children(NULL);
		for (std::vector<ConfigFileNode>::iterator iter = bindings.begin(); iter != bindings.end(); iter++)
		{
			ConfigFileNode binding_node = *iter;
			std::string name = file.Get_Name(binding_node);
			if (name == "binding")
			{
				const char* guid		= file.Get<const char*>("guid", binding_node, true);
				const char* iconset		= file.Get<const char*>("iconset", binding_node, true);
				const char* name		= file.Get<const char*>("name", binding_node, true);
				const char* os			= file.Get<const char*>("os", binding_node, true);
				bool        is_default	= file.Contains("default", binding_node, true) ? file.Get<bool>("default", binding_node, true) : false;

#if defined(PLATFORM_WIN32)
				const char* os_name = "win32";
#elif defined(PLATFORM_LINUX)
				const char* os_name = "linux";
#elif defined(PLATFORM_MACOS)
				const char* os_name = "macos";					
#endif

				std::vector<std::string> os_split;
				StringHelper::Split(os, ',', os_split);
				if (std::find(os_split.begin(), os_split.end(), os_name) != os_split.end())
				{
					JoystickMapping mapping;
					mapping.Name = name;
					mapping.Icon_Set = JoystickIconSet::Parse(iconset, JoystickIconSet::Xbox360);
					mapping.GUID = guid;
					mapping.Is_Default = is_default;

					bool Physical_Axis_Mapped[JoystickMapping::MAX_AXIS];
					bool Physical_Buttons_Mapped[JoystickMapping::MAX_BUTTONS];
					bool Physical_Hats_Mapped[JoystickMapping::MAX_HATS];

					for (int i = 0; i < JoystickMapping::MAX_AXIS; i++)
					{
						mapping.Axis_Mappings[i].Is_Mapped = false;
						mapping.Axis_Properties[i].Is_Enabled = true;
						Physical_Axis_Mapped[i] = false;
					}
					for (int i = 0; i < JoystickMapping::MAX_BUTTONS; i++)
					{
						mapping.Button_Mappings[i].Is_Mapped = false;
						mapping.Button_Properties[i].Is_Enabled = true;
						Physical_Buttons_Mapped[i] = false;
					}
					for (int i = 0; i < JoystickMapping::MAX_HATS; i++)
					{
						mapping.Hat_Mappings[i].Is_Mapped = false;
						mapping.Hat_Properties[i].Is_Enabled = true;
						Physical_Hats_Mapped[i] = false;
					}

#ifdef OPT_LOG_MAPPINGS
					DBG_LOG("Loading binding '%s'.", name);
#endif

					std::vector<ConfigFileNode> mapping_nodes = file.Get_Children(binding_node);
					for (std::vector<ConfigFileNode>::iterator mapping_iter = mapping_nodes.begin(); mapping_iter != mapping_nodes.end(); mapping_iter++)
					{
						ConfigFileNode mapping_node = *mapping_iter;
						std::string mapping_type = file.Get_Name(mapping_node);

						if (mapping_type == "bind_prop")
						{
							std::string physical_bind = file.Get<const char*>("physical", mapping_node, true);
							bool bEnabled = file.Get<bool>("enabled", mapping_node, true);
							if (physical_bind[0] == 'a')
							{
								int axis_index = StringHelper::Parse_Int(physical_bind.substr(1).c_str());
								mapping.Axis_Properties[axis_index].Is_Enabled = bEnabled;
							}
							else if (physical_bind[0] == 'b')
							{
								int button_index = StringHelper::Parse_Int(physical_bind.substr(1).c_str());
								mapping.Button_Properties[button_index].Is_Enabled = bEnabled;
							}
							else if (physical_bind[0] == 'h')
							{
								std::string sub_virtual = physical_bind.substr(1);
								int dot_offset = sub_virtual.find('.');
								int hat_index = StringHelper::Parse_Int(sub_virtual.substr(0, dot_offset).c_str());
								int hat_direction = StringHelper::Parse_Int(sub_virtual.substr(dot_offset + 1).c_str());

								int virtual_index = 0;

								if (hat_direction == 1)
								{
									virtual_index = (hat_index * 4) + 3;
								}
								else if (hat_direction == 2)
								{
									virtual_index = (hat_index * 4) + 0;
								}
								else if (hat_direction == 8)
								{
									virtual_index = (hat_index * 4) + 1;
								}
								else if (hat_direction == 4)
								{
									virtual_index = (hat_index * 4) + 2;
								}

								mapping.Hat_Properties[virtual_index].Is_Enabled = bEnabled;
							}
						}
						else if (mapping_type == "bind")
						{
							std::string virtual_value = file.Get<const char*>("virtual", mapping_node, true);

							int virtual_index = -1;
							int bind_type = 0;

							// Virtual axis.
							if (virtual_value == "LeftStickY")				{ virtual_index = 0; bind_type = 0; }
							else if (virtual_value == "LeftStickX")			{ virtual_index = 1; bind_type = 0; }
							else if (virtual_value == "RightStickY")		{ virtual_index = 2; bind_type = 0; }
							else if (virtual_value == "RightStickX")		{ virtual_index = 3; bind_type = 0; }
							else if (virtual_value == "LeftTrigger")		{ virtual_index = 4; bind_type = 0; }
							else if (virtual_value == "RightTrigger")		{ virtual_index = 5; bind_type = 0; }

							// Virtual buttons.
							else if (virtual_value == "A")					{ virtual_index = 0; bind_type = 1; }
							else if (virtual_value == "B")					{ virtual_index = 1; bind_type = 1; }
							else if (virtual_value == "X")					{ virtual_index = 2; bind_type = 1; }
							else if (virtual_value == "Y")					{ virtual_index = 3; bind_type = 1; }
							else if (virtual_value == "LB")					{ virtual_index = 4; bind_type = 1; }
							else if (virtual_value == "RB")					{ virtual_index = 5; bind_type = 1; }
							else if (virtual_value == "Back")				{ virtual_index = 6; bind_type = 1; }
							else if (virtual_value == "Start")				{ virtual_index = 7; bind_type = 1; }
							else if (virtual_value == "LeftStickClick")		{ virtual_index = 8; bind_type = 1; }
							else if (virtual_value == "RightStickClick")	{ virtual_index = 9; bind_type = 1; }
							else if (virtual_value == "Guide")				{ virtual_index = 10; bind_type = 1; }

							// Virtual POV's
							else if (virtual_value == "DPadRight")			{ virtual_index = 0; bind_type = 2; }
							else if (virtual_value == "DPadLeft")			{ virtual_index = 1; bind_type = 2; }
							else if (virtual_value == "DPadDown")			{ virtual_index = 2; bind_type = 2; }
							else if (virtual_value == "DPadUp")				{ virtual_index = 3; bind_type = 2; }

							if (virtual_index == -1)
							{
								if (virtual_value[0] == 'a')
								{
									bind_type = 0;
									virtual_index = StringHelper::Parse_Int(virtual_value.substr(1).c_str());
								}
								else if (virtual_value[0] == 'b')
								{
									bind_type = 1;
									virtual_index = StringHelper::Parse_Int(virtual_value.substr(1).c_str());
								}
								else if (virtual_value[0] == 'h')
								{
									bind_type = 2;

									std::string sub_virtual = virtual_value.substr(1);
									int dot_offset = sub_virtual.find('.');
									int hat_index = StringHelper::Parse_Int(sub_virtual.substr(0, dot_offset).c_str());
									int hat_direction = StringHelper::Parse_Int(sub_virtual.substr(dot_offset + 1).c_str());									

									// TODO: this is pretty bleh, what if we want more than 4 dirs?
									if (hat_direction == 1)
									{
										virtual_index = (hat_index * 4) + 3;
									}
									else if (hat_direction == 2)
									{
										virtual_index = (hat_index * 4) + 0;
									}
									else if (hat_direction == 8)
									{
										virtual_index = (hat_index * 4) + 1;
									}
									else if (hat_direction == 4)
									{
										virtual_index = (hat_index * 4) + 2;
									}
								}
							}

							std::string physical_bind = file.Get<const char*>("physical", mapping_node, true);

							JoystickMappingProperties* destination = NULL;

							if (bind_type == 0)
							{
#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("- Virtual Axis %i:", virtual_index);
#endif
								destination = &mapping.Axis_Mappings[virtual_index];
							}
							else if (bind_type == 1)
							{
#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("- Virtual Button %i:", virtual_index);
#endif
								destination = &mapping.Button_Mappings[virtual_index];
							}
							else if (bind_type == 2)
							{
#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("- Virtual Hat %i:", virtual_index);
#endif
								destination = &mapping.Hat_Mappings[virtual_index];
							}

							DBG_ASSERT_STR(!destination->Is_Mapped, "Invalid controller binding '%s', virtual axis '%i' is mapped more than once.", mapping.Name.c_str() ,virtual_index);

							if (physical_bind[0] == 'a')
							{
								destination->Is_Mapped = true;
								destination->Destination_Type = JoystickMappingInputType::Axis;
								destination->Destination_Index = StringHelper::Parse_Int(physical_bind.substr(1).c_str());

								DBG_ASSERT_STR(!Physical_Axis_Mapped[destination->Destination_Index], "Invalid controller binding '%s', physical axis '%i' is mapped more than once.", mapping.Name.c_str(), destination->Destination_Index);
								Physical_Axis_Mapped[destination->Destination_Index] = true;

#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("		Mapped to physical axis %i", destination->Destination_Index);
#endif
							}
							else if (physical_bind[0] == 'b')
							{
								destination->Is_Mapped = true;
								destination->Destination_Type = JoystickMappingInputType::Button;
								destination->Destination_Index = StringHelper::Parse_Int(physical_bind.substr(1).c_str());
								
								DBG_ASSERT_STR(!Physical_Buttons_Mapped[destination->Destination_Index], "Invalid controller binding '%s', physical button '%i' is mapped more than once.", mapping.Name.c_str(), destination->Destination_Index);
								Physical_Buttons_Mapped[destination->Destination_Index] = true;
								
#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("		Mapped to physical button %i", destination->Destination_Index);
#endif
							}
							else if (physical_bind[0] == 'h')
							{
								destination->Is_Mapped = true;
								destination->Destination_Type = JoystickMappingInputType::Hat;

								std::string sub_virtual = physical_bind.substr(1);
								int dot_offset = sub_virtual.find('.');
								int hat_index = StringHelper::Parse_Int(sub_virtual.substr(0, dot_offset).c_str());
								int hat_direction = StringHelper::Parse_Int(sub_virtual.substr(dot_offset + 1).c_str());

								// TODO: this is pretty bleh, what if we want more than 4 dirs?
								if (hat_direction == 1)
								{
									destination->Destination_Index = (hat_index * 4) + 3;
								}
								else if (hat_direction == 2)
								{
									destination->Destination_Index = (hat_index * 4) + 0;
								}
								else if (hat_direction == 8)
								{
									destination->Destination_Index = (hat_index * 4) + 1;
								}
								else if (hat_direction == 4)
								{
									destination->Destination_Index = (hat_index * 4) + 2;
								}

								DBG_ASSERT_STR(!Physical_Hats_Mapped[destination->Destination_Index], "Invalid controller binding '%s', physical hat '%i' is mapped more than once.", mapping.Name.c_str(), destination->Destination_Index);
								Physical_Hats_Mapped[destination->Destination_Index] = true;
								
#ifdef OPT_LOG_MAPPINGS
								DBG_LOG("		Mapped to physical hat %i", destination->Destination_Index);
#endif
							}
						}
					}

#ifdef OPT_LOG_MAPPINGS
					int unbound_virtual_axis = 0;
					int unbound_virtual_buttons = 0;
					int unbound_virtual_hats = 0;
					int unbound_physical_axis = 0;
					int unbound_physical_buttons = 0;
					int unbound_physical_hats = 0;

					for (int i = 0; i < JoystickMapping::MAX_AXIS; i++)
					{
						if (!mapping.Axis_Mappings[i].Is_Mapped)
						{
							unbound_virtual_axis++;
						}
						if (!Physical_Axis_Mapped[i])
						{
							unbound_physical_axis++;
						}
					}
					for (int i = 0; i < JoystickMapping::MAX_BUTTONS; i++)
					{
						if (!mapping.Button_Mappings[i].Is_Mapped)
						{
							unbound_virtual_buttons++;
						}
						if (!Physical_Buttons_Mapped[i])
						{
							unbound_physical_buttons++;
						}
					}
					for (int i = 0; i < JoystickMapping::MAX_HATS; i++)
					{
						if (!mapping.Hat_Mappings[i].Is_Mapped)
						{
							unbound_virtual_hats++;
						}
						if (!Physical_Hats_Mapped[i])
						{
							unbound_physical_hats++;
						}
					}

					DBG_LOG("Axis:%i/%i Buttons:%i/%i Hats:%i/%i Total:%i/%i", 
						unbound_virtual_axis, unbound_physical_axis,
						unbound_virtual_buttons, unbound_physical_buttons,
						unbound_virtual_hats, unbound_physical_hats,
						unbound_virtual_axis + unbound_virtual_buttons + unbound_virtual_hats,
						unbound_physical_axis + unbound_physical_buttons + unbound_physical_hats);
#endif

					// Fill in unassigned axis/button/hats.
					// This is a bit complicated as we need to check that any physical input we bind aren't bound by
					// another virtual input.

					// Say we have a joystick with 3 axis and 3 buttons and we use the following bindings:
					//   a0: b1
					//   a1: b2
					//   a2: ???
					//   b1: a0
					//   b2: a2
					//   b3: a1
					// When we bind the defaults we need to make sure when we bind a default for a2 we choose b3 as its the
					// only unallocated physical input.
					// We should try to keep types consistent (bind axis to axis and buttons to buttons) unless there is no other
					// available option.

					for (int i = 0; i < JoystickMapping::MAX_AXIS; i++)
					{
						if (!mapping.Axis_Mappings[i].Is_Mapped)
						{
							int Output_Index = -1;
							JoystickMappingInputType::Type Output_Index_Type = JoystickMappingInputType::Axis;

							// Arrays handed in in order of preference.
							Find_Next_Unbound_Mapping(
								Physical_Axis_Mapped, 
								JoystickMappingInputType::Axis,
								JoystickMapping::MAX_AXIS,

								Physical_Buttons_Mapped, 
								JoystickMappingInputType::Button,
								JoystickMapping::MAX_BUTTONS,

								Physical_Hats_Mapped, 
								JoystickMappingInputType::Hat,
								JoystickMapping::MAX_HATS,

								Output_Index,
								Output_Index_Type
							);


							if (Output_Index != -1)
							{
								mapping.Axis_Mappings[i].Is_Mapped = true;
								mapping.Axis_Mappings[i].Destination_Index = Output_Index;
								mapping.Axis_Mappings[i].Destination_Type = Output_Index_Type;
							}
							else
							{
								DBG_ASSERT_STR(false, "Invalid controller binding '%s', physical button is mapped more than once.", mapping.Name.c_str());
							}
						}
					}

					for (int i = 0; i < JoystickMapping::MAX_BUTTONS; i++)
					{
						if (!mapping.Button_Mappings[i].Is_Mapped)
						{
							int Output_Index = -1;
							JoystickMappingInputType::Type Output_Index_Type = JoystickMappingInputType::Button;

							// Arrays handed in in order of preference.
							Find_Next_Unbound_Mapping(
								Physical_Buttons_Mapped,
								JoystickMappingInputType::Button,
								JoystickMapping::MAX_BUTTONS,

								Physical_Hats_Mapped,
								JoystickMappingInputType::Hat,
								JoystickMapping::MAX_HATS,

								Physical_Axis_Mapped,
								JoystickMappingInputType::Axis,
								JoystickMapping::MAX_AXIS,

								Output_Index,
								Output_Index_Type
							);

							if (Output_Index != -1)
							{
								mapping.Button_Mappings[i].Is_Mapped = true;
								mapping.Button_Mappings[i].Destination_Index = Output_Index;
								mapping.Button_Mappings[i].Destination_Type = Output_Index_Type;
							}
							else
							{
								DBG_ASSERT_STR(false, "Invalid controller binding '%s', physical button is mapped more than once.", mapping.Name.c_str());
							}
						}
					}

					for (int i = 0; i < JoystickMapping::MAX_HATS; i++)
					{
						if (!mapping.Hat_Mappings[i].Is_Mapped)
						{
							int Output_Index = -1;
							JoystickMappingInputType::Type Output_Index_Type = JoystickMappingInputType::Button;

							// Arrays handed in in order of preference.
							Find_Next_Unbound_Mapping(
								Physical_Hats_Mapped,
								JoystickMappingInputType::Hat,
								JoystickMapping::MAX_HATS,

								Physical_Buttons_Mapped,
								JoystickMappingInputType::Button,
								JoystickMapping::MAX_BUTTONS,

								Physical_Axis_Mapped,
								JoystickMappingInputType::Axis,
								JoystickMapping::MAX_AXIS,

								Output_Index,
								Output_Index_Type
							);

							if (Output_Index != -1)
							{
								mapping.Hat_Mappings[i].Is_Mapped = true;
								mapping.Hat_Mappings[i].Destination_Index = Output_Index;
								mapping.Hat_Mappings[i].Destination_Type = Output_Index_Type;
							}
							else
							{
								DBG_ASSERT_STR(false, "Invalid controller binding '%s', physical button is mapped more than once.", mapping.Name.c_str());
							}
						}
					}

					g_input_mappings.push_back(mapping);
				}
				else
				{
#ifdef OPT_LOG_MAPPINGS
					DBG_LOG("Ignoring binding '%s', not valid for current OS.", name);
#endif
				}
			}
		}
	}
	else
	{
		DBG_LOG("Failed to open file '%s', unable to load joystick bindings.", url);
	}
}

JoystickMapping* Input::Get_Joystick_Mapping(std::string guid)
{
	for (std::vector<JoystickMapping>::iterator iter = g_input_mappings.begin(); iter != g_input_mappings.end(); iter++)
	{
		JoystickMapping& mapping = *iter;
		if (mapping.GUID == guid)
		{
			return &mapping;
		}
	}

	for (std::vector<JoystickMapping>::iterator iter = g_input_mappings.begin(); iter != g_input_mappings.end(); iter++)
	{
		JoystickMapping& mapping = *iter;
		if (mapping.Is_Default)
		{
			return &mapping;
		}
	}

	return NULL;
}

Input* Input::Create(int source_index)
{
	Input* new_input = NULL;

	if (source_index == 0)
	{
		for (int i = 0; i < MAX_INPUT_SOURCES; i++)
		{
			g_joystick_slots[i] = NULL;
		}

		g_dummy_joystick_state = new Null_JoystickState(0);

#ifdef STEAM_CONTROLLER_SUPPORT
		g_steam_input = new Steam_Input();
		g_steam_input->Scan_For_Devices();
#endif
	}

	// If we are not running with GUI we just return a
	// dummny renderer.
	if (*EngineOptions::nogui)
	{
		new_input = new Null_Input(source_index);
	}
	else
	{
#if defined(PLATFORM_WIN32)
		new_input = new Win32_Input(source_index);
#elif defined(PLATFORM_LINUX)
		new_input = new Linux_Input(source_index);
#elif defined(PLATFORM_MACOS)
		new_input = new MacOS_Input(source_index);
#else
		#error "Platform unsupported."
#endif
	}

	DBG_ASSERT_STR(g_input_sources[source_index] == NULL, "Attempt to create duplicate input source '%i'", source_index);
	new_input->m_index = source_index;
	g_input_sources[source_index] = new_input;
	m_input_source_count++;

	return new_input;
}

void Input::Destroy()
{	
	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		SAFE_DELETE(g_input_sources[i]);
	}
	SAFE_DELETE(g_steam_input);
	m_input_source_count = 0;
}

Input* Input::Get(int source_index)
{
	DBG_ASSERT_STR(g_input_sources[source_index] != NULL, "Attempt to retrieve invalid input source '%i'", source_index);
	return g_input_sources[source_index];
}

int Input::Get_Source_Count()
{
	return m_input_source_count;
}

void Input::Global_Scan_For_Devices()
{
	g_mark_for_device_scan = true;
}

void Input::Global_Tick(const FrameTime& time)
{
	if (g_mark_for_device_scan)
	{
#ifdef STEAM_CONTROLLER_SUPPORT
		g_steam_input->Scan_For_Devices();
#endif

		Input::Get()->Scan_For_Devices();

		g_mark_for_device_scan = false;
	}

	// This whole situation is pretty high-level dumbs, but eh, easier than restructing the joystick input system..
	std::vector<JoystickState*> known_controllers = g_input_sources[0]->Get_Known_Joysticks();

#ifdef STEAM_CONTROLLER_SUPPORT
	g_steam_input->Tick(time);

	// If we are doing steam controllers as well, add all the steam controllers to the 
	// start of the list (they take priority over other controllers).
	std::vector<JoystickState*> known_steam_controllers = g_steam_input->Get_Known_Joysticks();

	for (std::vector<JoystickState*>::iterator iter = known_steam_controllers.begin(); iter != known_steam_controllers.end(); iter++)
	{
		known_controllers.insert(known_controllers.begin(), *iter);
	}
#endif

	// Remove disconnected joysticks.
	std::vector<JoystickState*> active_joysticks;
	std::vector<JoystickState*> pending_joysticks;
	std::vector<int> empty_slots;

	for (unsigned int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		// Joystick in slot, if its been disconnected, get rid of it.
		if (g_joystick_slots[i] != NULL)
		{
			if (!g_joystick_slots[i]->Is_Connected())
			{
				DBG_LOG("[Joystick] Removed joystick %s from slot %i.", g_joystick_slots[i]->Get_UniqueID().c_str(), i);
				g_joystick_slots[i] = NULL;
			}
			else
			{
				active_joysticks.push_back(g_joystick_slots[i]);
			}
		}
		else
		{
			empty_slots.push_back(i);
		}
	}

	// Find all joysticks that want some space.
	for (unsigned int j = 0; j < known_controllers.size(); j++)
	{
		JoystickState* joystick = known_controllers[j];
		if (joystick->Is_Connected() &&
			std::find(active_joysticks.begin(), active_joysticks.end(), joystick) == active_joysticks.end())
		{
			pending_joysticks.push_back(joystick);
		}
	}

	// If any pending joysticks want to go back in a preferred slot, put them in.
	for (unsigned int i = 0; i < pending_joysticks.size(); i++)
	{
		JoystickState* joystick = pending_joysticks[i];

		std::vector<int>::iterator iter = std::find(empty_slots.begin(), empty_slots.end(), joystick->m_preferred_index);
		if (joystick->m_preferred_index >= 0 && iter != empty_slots.end())
		{
			g_joystick_slots[joystick->m_preferred_index] = joystick;

			DBG_LOG("[Joystick] Put joystick %s into preferred slot %i.", joystick->Get_UniqueID().c_str(), joystick->m_preferred_index);

			empty_slots.erase(iter);
			pending_joysticks.erase(pending_joysticks.begin() + i);
		}
	}

	// Try and fill any empty slots (that no joysticks have a preference for).
	for (unsigned int i = 0; i < empty_slots.size(); i++)
	{
		int slot = empty_slots[i];
		bool bHasPreference = false;

		for (unsigned int j = 0; j < known_controllers.size(); j++)
		{
			JoystickState* joystick = known_controllers[j];
			if (joystick->m_preferred_index == slot)
			{
				bHasPreference = true;
				break;
			}
		}

		if (!bHasPreference)
		{
			if (pending_joysticks.size() > 0)
			{
				g_joystick_slots[slot] = pending_joysticks[0];
				g_joystick_slots[slot]->m_preferred_index = slot;

				DBG_LOG("[Joystick] Put joystick %s into empty and unpreferred slot %i.", g_joystick_slots[slot]->Get_UniqueID().c_str(), slot);

				pending_joysticks.erase(pending_joysticks.begin());
				empty_slots.erase(empty_slots.begin() + i);
				i--;
			}
		}
	}

	// Still got pending joysticks? Put them in any left over empty slots.
	while (pending_joysticks.size() > 0 && empty_slots.size() > 0)
	{
		int slot = empty_slots[0];
		JoystickState* joystick = pending_joysticks[0];

		g_joystick_slots[slot] = joystick;
		g_joystick_slots[slot]->m_preferred_index = slot;

		DBG_LOG("[Joystick] Put joystick %s slot %i.", g_joystick_slots[slot]->Get_UniqueID().c_str(), slot);

		empty_slots.erase(empty_slots.begin());
		pending_joysticks.erase(pending_joysticks.begin());
	}
}

JoystickState* Input::Get_Joystick_State()
{
	if (g_joystick_slots[m_index] == NULL)
	{
		return g_dummy_joystick_state;
	}
	return g_joystick_slots[m_index];
}

Input::Input()
{
	memset(m_binding_state, 0, sizeof(bool) * OutputBindings::COUNT);
	memset(m_previous_binding_state, 0, sizeof(bool) * OutputBindings::COUNT);
}

void Input::Update_Bindings()
{	
	memcpy(m_previous_binding_state, m_binding_state, sizeof(bool) * OutputBindings::COUNT);
	memset(m_binding_state, 0, sizeof(bool) * OutputBindings::COUNT);
	
	// If display is not active, we don't want this input..
	GfxDisplay* display = GfxDisplay::Get();
	if (display != NULL && !display->Is_Active())
	{
		memset(m_previous_binding_state, 0, sizeof(bool) * OutputBindings::COUNT);
		memset(m_binding_state, 0, sizeof(bool) * OutputBindings::COUNT);
		return;
	}

	for (std::vector<InputBind>::iterator iter = m_bindings.begin(); iter != m_bindings.end(); iter++)
	{
		InputBind& bind = *iter;

		if (Is_Down(bind.Input))
		{
			m_binding_state[(int)bind.Output] = true;
		}
	}

	for (int i = 0; i < OutputBindings::COUNT; i++)
	{
		if (!m_binding_state[i])
		{
			m_binding_state[i] = Is_Action_Down((OutputBindings::Type)i);
			if (m_binding_state[i])
			{
			/*	const char* Result = "";
				OutputBindings::Type Type = (OutputBindings::Type)i;
				OutputBindings::ToString(Type, Result);
				DBG_LOG("Pressed %s", Result);*/
			}
		}
	}
}

InputBindings::Type Input::Output_To_Active_Input(OutputBindings::Type type, InputBindingType::Type key_type, JoystickState** state)
{
	InputBindings::Type input = InputBindings::INVALID;
	float last_source_input = FLT_MAX;

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{	
		Input* in = Input::Get(i);

		for (std::vector<InputBind>::iterator iter = in->m_bindings.begin(); iter != in->m_bindings.end(); iter++)
		{
			InputBind& bind = *iter;
			if (bind.Output == type)
			{		
				if (key_type == InputBindingType::Joystick &&
					(bind.Input <= InputBindings::JOYSTICK_START ||
					bind.Input >= InputBindings::JOYSTICK_END))
				{
					continue;
				}
				else if (key_type == InputBindingType::Keyboard &&
					(bind.Input <= InputBindings::KEYBOARD_START ||
					bind.Input >= InputBindings::KEYBOARD_END))
				{
					continue;
				}

				float input_time = in->Last_Source_Input(bind.Input);

				if (input == InputBindings::INVALID ||
					input_time < last_source_input)
				{
					input = bind.Input;
					last_source_input = input_time;

					if (state != NULL)
					{
						if (bind.Input >= InputBindings::JOYSTICK_START && bind.Input <= InputBindings::JOYSTICK_END)
						{
							*state = in->Get_Joystick_State();
						}
					}
				}
			}
		}
	}

//	const char* str = NULL;
//	InputBindings::To_String(input, str);
//	DBG_LOG("Time=%f Input=%s", last_source_input, str);

	return input;
}

JoystickIconSet::Type Input::Get_Active_Joystick_Icon_Set()
{
	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		if (Get(i)->Get_Joystick_State()->Is_Connected())
		{
			return Get(i)->Get_Joystick_State()->Get_Icon_Set();			
		}
	}
	return JoystickIconSet::Xbox360;
}

float Input::Last_Source_Input(InputBindings::Type type)
{
	if (type < InputBindings::KEYBOARD_END)
	{
		return Get_Keyboard_State()->Time_Since_Last_Input();
	}
	else if (type < InputBindings::MOUSE_END)
	{
		return Get_Mouse_State()->Time_Since_Last_Input();
	}
	else if (type < InputBindings::JOYSTICK_END && Get_Joystick_State()->Is_Connected())
	{
		return Get_Joystick_State()->Time_Since_Last_Input();
	}

	return FLT_MAX;
}

void Input::Unbind_All()
{
	m_bindings.clear();
}

void Input::Bind(InputBindings::Type input, OutputBindings::Type output)
{
	m_bindings.push_back(InputBind(input, output));
}

bool Input::Is_Down(InputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	if (type < InputBindings::KEYBOARD_END)
	{
		return Get_Keyboard_State()->Is_Key_Down(type);
	}
	else if (type < InputBindings::MOUSE_END)
	{
		return Get_Mouse_State()->Is_Button_Down(type);
	}
	else if (type < InputBindings::JOYSTICK_END)
	{
		return Get_Joystick_State()->Is_Key_Down(type);
	}
	else
	{
		DBG_ASSERT_STR(false, "Invalid input key.");
	}

	return false;
}

bool Input::Was_Down(InputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	if (type < InputBindings::KEYBOARD_END)
	{
		return Get_Keyboard_State()->Was_Key_Down(type);
	}
	else if (type < InputBindings::MOUSE_END)
	{
		return Get_Mouse_State()->Was_Button_Down(type);
	}
	else if (type < InputBindings::JOYSTICK_END)
	{
		return Get_Joystick_State()->Was_Key_Down(type);
	}
	else
	{
		DBG_ASSERT_STR(false, "Invalid input key.");
	}

	return false;
}

bool Input::Was_Pressed(InputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	if (type < InputBindings::KEYBOARD_END)
	{
		return Get_Keyboard_State()->Was_Key_Pressed(type);
	}
	else if (type < InputBindings::MOUSE_END)
	{
		return Get_Mouse_State()->Was_Button_Clicked(type);
	}
	else if (type < InputBindings::JOYSTICK_END)
	{
		return Get_Joystick_State()->Was_Key_Pressed(type);
	}
	else
	{
		DBG_ASSERT_STR(false, "Invalid input key.");
	}

	return false;
}

bool Input::Is_Action_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

#ifdef STEAM_CONTROLLER_SUPPORT
/*	if (this != g_steam_input)
	{
		if (g_steam_input->Was_Action_Down(type))
		{
			return true;
		}
	}*/
#endif

	if (Get_Keyboard_State()->Is_Action_Down(type) ||
		Get_Mouse_State()->Is_Action_Down(type) ||
		Get_Joystick_State()->Is_Action_Down(type))
	{
		return true;
	}

	return false;
}

bool Input::Was_Action_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

#ifdef STEAM_CONTROLLER_SUPPORT
/*	if (this != g_steam_input)
	{
		if (g_steam_input->Was_Action_Down(type))
		{
			return true;
		}
	}*/
#endif

	if (Get_Keyboard_State()->Was_Action_Down(type) ||  
		Get_Mouse_State()->Was_Action_Down(type) || 
		Get_Joystick_State()->Was_Action_Down(type))
	{
		return true;
	}

	return false;
}

bool Input::Was_Action_Pressed(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

#ifdef STEAM_CONTROLLER_SUPPORT
/*	if (this != g_steam_input)
	{
		if (g_steam_input->Was_Action_Pressed(type))
		{
			return true;
		}
	}*/
#endif

	if (Get_Keyboard_State()->Was_Action_Pressed(type) ||
		Get_Mouse_State()->Was_Action_Pressed(type) ||
		Get_Joystick_State()->Was_Action_Pressed(type))
	{
		return true;
	}

	return false;
}

bool Input::Is_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	return m_binding_state[(int)type];
}

bool Input::Was_Down(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	return m_previous_binding_state[(int)type];
}

bool Input::Was_Pressed(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	return m_binding_state[(int)type] && !m_previous_binding_state[(int)type];
}

bool Input::Was_Pressed_By_Any(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		if (Input::Get(i)->Was_Pressed(type))
		{
			return true;
		}
	}

	return false;
}

bool Input::Is_Down_By_Any(OutputBindings::Type type)
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return false;
	}

	for (int i = 0; i < MAX_INPUT_SOURCES; i++)
	{
		if (Input::Get(i)->Is_Down(type))
		{
			return true;
		}
	}

	return false;
}

std::string Input::Read_Character()
{
	if (OnlinePlatform::Get()->Is_Dialog_Open())
	{
		return "";
	}

	std::string result = Get_Keyboard_State()->Read_Character();
	if (result != "") 
	{
		return result;
	}

	result = Get_Joystick_State()->Read_Character();
	if (result != "") 
	{
		return result;
	}

	return "";
}

void Input::Flush_Character_Stack()
{
	Get_Keyboard_State()->Flush_Character_Stack();
	Get_Joystick_State()->Flush_Character_Stack();
}