// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_BOOL_OPTION_
#define _ENGINE_OPTIONS_BOOL_OPTION_

#include "Engine/Options/Option.h"

class BoolOption : public Option
{
	MEMORY_ALLOCATOR(BoolOption, "Engine");

private:
	bool m_data;

public:
	BoolOption(bool default_value, const char* name, const char* description, OptionFlags::Type flags);

	bool Parse(const char* value);
	std::string To_String();
	
	unsigned int Get_Type_ID();

	bool Serialize(BinaryStream& stream);
	bool Deserialize(BinaryStream& stream, unsigned int version);

	bool& operator*();

};

#endif

