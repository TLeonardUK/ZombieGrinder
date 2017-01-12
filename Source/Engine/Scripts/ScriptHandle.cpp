// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/ScriptFactory.h"
#include "Engine/Scripts/ScriptHandle.h"
#include "Engine/Scripts/Script.h"

ScriptHandle::ScriptHandle(const char* url, Script* layout)
	: m_url(url)
	, m_script(layout)
{
	Add_Reload_Trigger_File(url);
}

ScriptHandle::~ScriptHandle()
{
	SAFE_DELETE(m_script);
}
	
void ScriptHandle::Reload()
{
	Script* new_layout = ScriptFactory::Load_Without_Handle(m_url.c_str());
	if (new_layout != NULL)
	{
		SAFE_DELETE(m_script);			
		m_script = new_layout;

		DBG_LOG("Reloaded changed script: %s", m_url.c_str());
	}
}

Script* ScriptHandle::Get()
{
	return m_script;
}

std::string ScriptHandle::Get_URL()
{
	return m_url;
}