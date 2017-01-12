// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Localise/LanguageFactory.h"
#include "Engine/Localise/LanguageHandle.h"
#include "Engine/Localise/Language.h"

LanguageHandle::LanguageHandle(const char* url, Language* bank)
	: m_url(url)
	, m_bank(bank)
{
	Add_Reload_Trigger_File(url);
}

LanguageHandle::~LanguageHandle()
{
	SAFE_DELETE(m_bank);
}
	
void LanguageHandle::Reload()
{
	Language* new_sound = LanguageFactory::Load_Without_Handle(m_url.c_str());
	if (new_sound != NULL)
	{
		SAFE_DELETE(m_bank);			
		m_bank = new_sound;

		DBG_LOG("Reloaded changed language: %s", m_url.c_str());
	}
}

Language* LanguageHandle::Get()
{
	return m_bank;
}

std::string LanguageHandle::Get_URL()
{
	return m_url;
}
