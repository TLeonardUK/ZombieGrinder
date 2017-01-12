// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_INT_OPTION_
#define _ENGINE_OPTIONS_INT_OPTION_

#include "Engine/Options/Option.h"

class IntOption : public Option
{
	MEMORY_ALLOCATOR(IntOption, "Engine");

private:
	int m_data;

public:
	IntOption(int default_value, const char* name, const char* description, OptionFlags::Type flags);

	bool Parse(const char* value);
	std::string To_String();

	unsigned int Get_Type_ID();

	bool Serialize(BinaryStream& stream);
	bool Deserialize(BinaryStream& stream, unsigned int version);

	int& operator*();

};

#endif

