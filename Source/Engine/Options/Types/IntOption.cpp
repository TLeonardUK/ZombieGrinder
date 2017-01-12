// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Types/IntOption.h"

#include "Generic/Helper/StringHelper.h"

IntOption::IntOption(int default_value, const char* name, const char* description, OptionFlags::Type flags)
	: Option(name, description, flags)
	, m_data(default_value)
{
}

bool IntOption::Parse(const char* value)
{
	m_data = atoi(value);
	return true;
}

std::string IntOption::To_String()
{
	return StringHelper::To_String(m_data);
}

int& IntOption::operator*()
{
	return m_data;
}

unsigned int IntOption::Get_Type_ID()
{
	static unsigned int id = StringHelper::Hash("IntOption");
	return id;
}

bool IntOption::Serialize(BinaryStream& stream)
{
	stream.Write<int>(m_data);
	return true;
}

bool IntOption::Deserialize(BinaryStream& stream, unsigned int version)
{
	m_data = stream.Read<int>();
	return true;
}