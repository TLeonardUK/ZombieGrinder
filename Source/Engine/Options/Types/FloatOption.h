// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_OPTIONS_FLOAT_OPTION_
#define _ENGINE_OPTIONS_FLOAT_OPTION_

#include "Engine/Options/Option.h"

class FloatOption : public Option
{
	MEMORY_ALLOCATOR(FloatOption, "Engine");

private:
	float m_data;

public:
	FloatOption(float default_value, const char* name, const char* description, OptionFlags::Type flags);

	bool Parse(const char* value);
	std::string To_String();
	
	unsigned int Get_Type_ID();

	bool Serialize(BinaryStream& stream);
	bool Deserialize(BinaryStream& stream, unsigned int version);

	float& operator*();

};

#endif

