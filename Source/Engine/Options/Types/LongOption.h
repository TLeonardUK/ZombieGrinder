// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_LONG_OPTION_
#define _ENGINE_OPTIONS_LONG_OPTION_

#include "Engine/Options/Option.h"

class LongOption : public Option
{
	MEMORY_ALLOCATOR(LongOption, "Engine");

private:
	u64 m_data;

public:
	LongOption(u64 default_value, const char* name, const char* description, OptionFlags::Type flags);

	bool Parse(const char* value);
	std::string To_String();

	unsigned int Get_Type_ID();

	bool Serialize(BinaryStream& stream);
	bool Deserialize(BinaryStream& stream, unsigned int version);

	u64& operator*();

};

#endif

