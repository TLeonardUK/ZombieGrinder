// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef CONFIG_INC_FILE
#error "CONFIG_INC_FILE must be defined when including this file!"
#endif

// This file will deserialise all CONFIG_VALUE settings in file CONFIG_INC_FILE as member variables.
// This can be used in conjunction with ConfigFileDefiner.inc to automatically deserialise config settings.

// Define different configuration value types.
#define CONFIG_VALUE(type, name, serial_name, default)	if (file.Contains(serial_name)) \
															name = file.Get<type>(serial_name); \
														else \
															name = default;

#define BEGIN_CONFIG_LIST(type, name, serial_name)	{ \
														std::vector<ConfigFileNode> nodes = file.Get<std::vector<ConfigFileNode>>(serial_name); \
														std::vector<type> defaults; \
														name.clear(); \
														for (std::vector<ConfigFileNode>::iterator iter = nodes.begin(); iter != nodes.end(); iter++) \
														{ \
															ConfigFileNode& node = *iter; \
															type value;

#define CONFIG_LIST_ELEMENT(type, name, serial_name)		if (file.Contains(serial_name, node, true))	\
															{ \
																value.name = file.Get<type>(serial_name, node, true); \
															}

#define CONFIG_LIST_ELEMENT_ENUM(type, name, serial_name)	if (file.Contains(serial_name, node, true))	\
															{ \
																const char* v = file.Get<const char*>(serial_name, node, true);\
																type::Type result; \
																value.name = (type::Type)-1; \
																if (type::Parse(v, result)) \
																	value.name = result; \
															}

#define END_CONFIG_LIST(type, name, serial_name)			name.push_back(value); \
														} \
														if (nodes.size() <= 0 && defaults.size() > 0) \
															name.insert(name.end(), defaults.begin(), defaults.end()); \
														\
													}
#define CONFIG_LIST_DEFAULT(value)					defaults.push_back(value);

// Do the magic!
#include CONFIG_INC_FILE

// Undefine everything!
#undef CONFIG_VALUE
#undef BEGIN_CONFIG_LIST
#undef CONFIG_LIST_DEFAULT
#undef CONFIG_LIST_ELEMENT
#undef CONFIG_LIST_ELEMENT_ENUM
#undef END_CONFIG_LIST