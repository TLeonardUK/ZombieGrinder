// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================

#ifndef CONFIG_INC_FILE
#error "CONFIG_INC_FILE must be defined when including this file!"
#endif

// This file will define all CONFIG_VALUE settings in file CONFIG_INC_FILE as member variables.
// This can be used in conjunction with ConfigFileDeserializer.inc to automatically deserialise config settings.

// Define different configuration value types.
#define CONFIG_VALUE(type, name, serial_name, default)	type name;

#define BEGIN_CONFIG_LIST(type, name, serial_name)		//struct type {
#define CONFIG_LIST_ELEMENT(type, name, serial_name)	//	type name;
#define CONFIG_LIST_ELEMENT_ENUM(type, name, serial_name)//	type::Type name;
#define CONFIG_LIST_DEFAULT(value)
#define END_CONFIG_LIST(type, name, serial_name)		std::vector<type> name;

// Do the magic!
#include CONFIG_INC_FILE

// Undefine everything!
#undef CONFIG_VALUE
#undef BEGIN_CONFIG_LIST
#undef CONFIG_LIST_DEFAULT
#undef CONFIG_LIST_ELEMENT
#undef CONFIG_LIST_ELEMENT_ENUM
#undef END_CONFIG_LIST