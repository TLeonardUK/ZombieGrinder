// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Localise/Language.h"
#include "Engine/IO/StreamFactory.h"
#include "Engine/Resources/ResourceFactory.h"

#include "Generic/Helper/StringHelper.h"

Language::Language()
	: Short_Name("")
	, Long_Name("")
	, URL(NULL)
{
}

Language::~Language()
{
	SAFE_DELETE(m_resource_data);
}

bool Language::Load_Compiled_Config(CompiledLanguageHeader* config)
{
	m_resource_data = config;

	Long_Name = config->strings[config->long_name_string_index].value;
	Short_Name = config->strings[config->short_name_string_index].value;

	for (unsigned int i = 0; i < config->string_count; i++)
	{
		Strings.Set(config->strings[i].name_hash, config->strings[i].value);
	}

	DBG_LOG("Loaded langauge: %s (%s)", Long_Name, Short_Name);

	return true;
}

void Language::Add_Sub_Language(Language* language)
{
	DBG_LOG("Mounted sub-langauge file onto primary language file: %s (%s)", language->Long_Name, language->Short_Name);

	Sub_Languages.push_back(language);
}

void Language::Refresh_Strings()
{
	Language_Strings.clear();

	LanguageStrings str;
	str.Priority = Priority;
	str.Strings = &Strings;
	str.Base = this;
	Language_Strings.push_back(str);

	for (unsigned int i = 0; i < Sub_Languages.size(); i++)
	{
		Language* other = Sub_Languages[i];

		LanguageStrings substr;
		substr.Priority = other->Priority;
		substr.Strings = &other->Strings;
		substr.Base = other;
		Language_Strings.push_back(substr);
	}

	std::sort(Language_Strings.begin(), Language_Strings.end(), &LanguageStrings::Sort_Predicate);
}