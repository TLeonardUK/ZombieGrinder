// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_STRING_OPTION_
#define _ENGINE_OPTIONS_STRING_OPTION_

#include "Engine/Options/Option.h"

class StringOption : public Option
{
	MEMORY_ALLOCATOR(StringOption, "Engine");

private:
	std::string m_data;

public:
	StringOption(std::string default_value, const char* name, const char* description, OptionFlags::Type flags);

	bool Parse(const char* value);
	std::string To_String();
	
	unsigned int Get_Type_ID();

	bool Serialize(BinaryStream& stream);
	bool Deserialize(BinaryStream& stream, unsigned int version);

	std::string& operator*();

};

#endif

