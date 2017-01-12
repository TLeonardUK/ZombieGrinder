// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Types/FloatOption.h"

#include "Generic/Helper/StringHelper.h"

FloatOption::FloatOption(float default_value, const char* name, const char* description, OptionFlags::Type flags)
	: Option(name, description, flags)
	, m_data(default_value)
{
}

bool FloatOption::Parse(const char* value)
{
	m_data = (float)atof(value);
	return true;
}

std::string FloatOption::To_String()
{
	return StringHelper::To_String(m_data);
}

float& FloatOption::operator*()
{
	return m_data;
}

unsigned int FloatOption::Get_Type_ID()
{
	static unsigned int id = StringHelper::Hash("FloatOption");
	return id;
}

bool FloatOption::Serialize(BinaryStream& stream)
{
	stream.Write<float>(m_data);
	return true;
}

bool FloatOption::Deserialize(BinaryStream& stream, unsigned int version)
{
	m_data = stream.Read<float>();
	return true;
}