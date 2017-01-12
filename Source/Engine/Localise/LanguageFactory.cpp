// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Localise/LanguageFactory.h"

#include "Engine/Resources/ResourceFactory.h"
#include "Engine/IO/PatchedBinaryStream.h"

#include "Engine/Platform/Platform.h"

#include "Engine/Config/ConfigFile.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<LanguageHandle*, int> LanguageFactory::m_loaded_banks;

LanguageHandle* LanguageFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_banks.Contains(url_hash))
	{
		DBG_LOG("Loaded language from cache: %s", url);
		return m_loaded_banks.Get(url_hash);
	}

	// Try and load texture!
	Language* sound = Load_Without_Handle(url);
	if (sound != NULL)
	{		
		LanguageHandle* handle = new LanguageHandle(url, sound);

		m_loaded_banks.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Language* LanguageFactory::Load_Without_Handle(const char* url)
{
	Language* result = Try_Load(url);
	if (result != NULL)
	{
		DBG_LOG("Loaded language: %s", url);
		return result;
	}

	DBG_LOG("Failed to load language: %s", url);
	return NULL;
}

Language* LanguageFactory::Try_Load(const char* url)
{
	Platform* platform = Platform::Get();

	Stream* chunk_stream = ResourceFactory::Get()->Open(url, StreamMode::Read);
	if (chunk_stream == NULL)
	{
		return NULL;
	}

	PatchedBinaryStream* stream = new PatchedBinaryStream(chunk_stream, StreamMode::Read);
	SAFE_DELETE(chunk_stream);

	CompiledLanguageHeader* header = (CompiledLanguageHeader*)stream->Take_Data();

	// Load compiled config.
	Language* language = new Language();
	if (!language->Load_Compiled_Config(header))
	{
		SAFE_DELETE(language);
		SAFE_DELETE(stream);

		DBG_LOG("Failed to load language from '%s'.", url);
		return NULL;
	}
	
	stream->Close();
	SAFE_DELETE(stream);

	DBG_LOG("Finished loading language from '%s'.", url);
	return language;
}

void LanguageFactory::Dispose()
{
	for (ThreadSafeHashTable<LanguageHandle*, int>::Iterator iter = m_loaded_banks.Begin(); iter != m_loaded_banks.End(); iter++)
	{
		LanguageHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_banks.Clear();
}

std::vector<LanguageHandle*> LanguageFactory::Get_Languages()
{
	std::vector<LanguageHandle*> result;
	for (ThreadSafeHashTable<LanguageHandle*, int>::Iterator iter = m_loaded_banks.Begin(); iter != m_loaded_banks.End(); iter++)
	{
		LanguageHandle* handle = *iter;
		result.push_back(handle);
	}
	return result;
}