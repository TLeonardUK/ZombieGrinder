// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scripts/ScriptFactory.h"
#include "Engine/Scripts/Script.h"
#include "Engine/Scripts/ScriptHandle.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<ScriptHandle*, int>	ScriptFactory::m_loaded_scripts;

ScriptFactory::ScriptFactory()
{
}

ScriptFactory::~ScriptFactory()
{
}

ScriptHandle* ScriptFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_scripts.Contains(url_hash))
	{
		DBG_LOG("Loaded script from cache: %s", url);
		return m_loaded_scripts.Get(url_hash);
	}

	// Try and load texture!
	Script* layout = Load_Without_Handle(url);
	if (layout != NULL)
	{		
		ScriptHandle* handle = new ScriptHandle(url, layout);

		m_loaded_scripts.Set(url_hash, handle);
		
		DBG_LOG("Loaded UI script: %s", url);

		return handle;
	}
	else
	{
		DBG_LOG("Failed to load UI script: %s", url);
	}

	return NULL;
}

Script* ScriptFactory::Load_Without_Handle(const char* url)
{
	return Script::Load(url);
}

void ScriptFactory::Dispose()
{
	for (ThreadSafeHashTable<ScriptHandle*, int>::Iterator iter = m_loaded_scripts.Begin(); iter != m_loaded_scripts.End(); iter++)
	{
		ScriptHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_scripts.Clear();
}
