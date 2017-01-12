// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Types/BoolOption.h"
#include "Generic/Helper/StringHelper.h"

BoolOption::BoolOption(bool default_value, const char* name, const char* description, OptionFlags::Type flags)
	: Option(name, description, flags)
	, m_data(default_value)
{
}

bool BoolOption::Parse(const char* value)
{
	if (stricmp(value, "true") == 0 ||
		stricmp(value, "1") == 0)
	{
		m_data = true;
		return true;
	}
	else if (stricmp(value, "false") == 0 ||
			 stricmp(value, "0") == 0)
	{
		m_data = false;
		return true;
	}

	return false;
}

std::string BoolOption::To_String()
{
	return m_data ? "1" : "0";
}

bool& BoolOption::operator*()
{
	return m_data;
}

unsigned int BoolOption::Get_Type_ID()
{
	static unsigned int id = StringHelper::Hash("BoolOption");
	return id;
}

bool BoolOption::Serialize(BinaryStream& stream)
{
	stream.Write<char>(m_data);
	return true;
}

bool BoolOption::Deserialize(BinaryStream& stream, unsigned int version)
{
	m_data = (stream.Read<char>() != 0);
	return true;
}