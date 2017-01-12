// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Types/LongOption.h"

#include "Generic/Helper/StringHelper.h"

#include <string>

LongOption::LongOption(u64 default_value, const char* name, const char* description, OptionFlags::Type flags)
	: Option(name, description, flags)
	, m_data(default_value)
{
}

bool LongOption::Parse(const char* value)
{
	// Note: stoull is C++0x only ;_;
#if defined(LANGUAGE_STANDARD_CPP11) || defined(PLATFORM_WIN32)
	m_data = std::stoull(value);
#else
	m_data = strtoull(value, NULL, 10);
#endif
	return true;
}

std::string LongOption::To_String()
{
	return StringHelper::To_String(m_data);
}

u64& LongOption::operator*()
{
	return m_data;
}

unsigned int LongOption::Get_Type_ID()
{
	static unsigned int id = StringHelper::Hash("LongOption");
	return id;
}

bool LongOption::Serialize(BinaryStream& stream)
{
	stream.Write<u64>(m_data);
	return true;
}

bool LongOption::Deserialize(BinaryStream& stream, unsigned int version)
{
	m_data = stream.Read<u64>();
	return true;
}