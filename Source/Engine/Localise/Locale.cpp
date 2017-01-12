// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Localise/Locale.h"
#include "Engine/Localise/Language.h"
#include "Engine/Localise/LanguageHandle.h"
#include "Engine/IO/StreamFactory.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

Locale::Locale()
	: m_current_language(NULL)
{
}

Locale::~Locale()
{
}

bool Locale::Change_Language(const char* name)
{
	LanguageHandle* lang = ResourceFactory::Get()->Get_Language(name);
	return Change_Language(lang);
}

bool Locale::Change_Language(LanguageHandle* language)
{
	if (language == NULL)
	{
		return false;
	}

	DBG_LOG("Changed to langauge: %s (%s)", language->Get()->Long_Name, language->Get()->Short_Name);

	m_current_language = language;
	return true;
}

const char* Locale::Get_String(std::string& id, bool return_null)
{
	return Get_String(id.c_str(), return_null);
}

const char* Locale::Get_String(const char* id, bool return_null)
{
	if (id[0] == '#')
	{
		return Get_String(StringHelper::Hash(id + 1), return_null);
	}
	else
	{
		return id;
	}
}

const char* Locale::Get_String(int id, bool return_null)
{
	DBG_ASSERT(m_current_language != NULL);

	const char* str = NULL;

	for (unsigned int i = 0; i < m_current_language->Get()->Language_Strings.size() && str == NULL; i++)
	{
		LanguageStrings& strings = m_current_language->Get()->Language_Strings[i];
		str = strings.Strings->Get(id);;
	}

	if (str == NULL && return_null == false)
	{
		DBG_LOG("Missing string for hash code : 0x%08x", id);
		str = "##_MISSING_STRING_##";
		m_current_language->Get()->Strings.Set(id, "##_MISSING_STRING_##");
	}
	return str;
}