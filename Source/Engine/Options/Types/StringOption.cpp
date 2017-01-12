// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Types/StringOption.h"

#include "Generic/Helper/StringHelper.h"

StringOption::StringOption(std::string default_value, const char* name, const char* description, OptionFlags::Type flags)
	: Option(name, description, flags)
	, m_data(default_value)
{
}

bool StringOption::Parse(const char* value)
{
	m_data = value;
	return true;
}

std::string StringOption::To_String()
{
	return StringHelper::To_String(m_data);
}

std::string& StringOption::operator*()
{
	return m_data;
}

unsigned int StringOption::Get_Type_ID()
{
	static unsigned int id = StringHelper::Hash("StringOption");
	return id;
}

bool StringOption::Serialize(BinaryStream& stream)
{
	stream.Write<std::string>(m_data);
	return true;
}

bool StringOption::Deserialize(BinaryStream& stream, unsigned int version)
{
	m_data = stream.Read<std::string>();
	return true;
}