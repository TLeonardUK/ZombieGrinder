// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Options/Option.h"

#include "Generic/Helper/StringHelper.h"

Option::Option(const char* name, const char* description, OptionFlags::Type flags)
	: m_name(name)
	, m_description(description)
	, m_flags(flags)
{
	m_name_hash = StringHelper::Hash(m_name);
}
