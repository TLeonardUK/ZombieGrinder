// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef CONFIG_INC_FILE
#error "CONFIG_INC_FILE must be defined when including this file!"
#endif

// This file will deserialise all CONFIG_VALUE settings in file CONFIG_INC_FILE as member variables.
// This can be used in conjunction with ConfigFileDefiner.inc to automatically deserialise config settings.

// Define different configuration value types.
#define CONFIG_VALUE(type, name, serial_name, default) file.Set<type>(serial_name, name);

#define BEGIN_CONFIG_LIST(type, name, serial_name)			{ \
																	file.Clear_Nodes(serial_name); \
																	std::vector<type> values = name; \
																	for (std::vector<type>::iterator iter = values.begin(); iter != values.end(); iter++) \
																	{ \
																		type val = *iter; \
																		ConfigFileNode node = file.New_Node(serial_name); 

#define CONFIG_LIST_ELEMENT(type, name, serial_name)					file.Set<type>(serial_name, val.name, node, true);

#define CONFIG_LIST_ELEMENT_ENUM(type, name, serial_name)				{ \
																			const char* result; \
																			if (type::ToString(val.name, result)) \
																			{ \
																				file.Set<const char*>(serial_name, result, node, true); \
																			} \
																		}

#define END_CONFIG_LIST(type, name, serial_name)					} \
															}
#define CONFIG_LIST_DEFAULT(value)

// Do the magic!
#include CONFIG_INC_FILE

// Undefine everything!
#undef CONFIG_VALUE
#undef BEGIN_CONFIG_LIST
#undef CONFIG_LIST_DEFAULT
#undef CONFIG_LIST_ELEMENT
#undef CONFIG_LIST_ELEMENT_ENUM
#undef END_CONFIG_LIST