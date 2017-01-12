// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Text/FontFactory.h"
#include "Engine/Renderer/Text/Font.h"
#include "Engine/Renderer/Text/FontHandle.h"

#include "Generic/Helper/StringHelper.h"

LinkedList<FontFactory*>	FontFactory::m_factories;
ThreadSafeHashTable<FontHandle*, int>	FontFactory::m_loaded_fonts;

FontHandle* FontFactory::Load(const char* url, FontFlags::Type flags)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_fonts.Contains(url_hash))
	{
		DBG_LOG("Loaded font from cache: %s", url);
		return m_loaded_fonts.Get(url_hash);
	}

	// Try and load texture!
	Font* font = Load_Without_Handle(url, flags);
	if (font != NULL)
	{		
		FontHandle* handle = new FontHandle(url, flags, font);

		m_loaded_fonts.Set(url_hash, handle);

		return handle;
	}

	return NULL;
}

Font* FontFactory::Load_Without_Handle(const char* url, FontFlags::Type flags)
{
	for (LinkedList<FontFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		FontFactory* factory = *iter;
		Font* result = factory->Try_Load(url, flags);
		if (result != NULL)
		{
			DBG_LOG("Loaded font: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load font: %s", url);
	return NULL;
}

FontFactory::FontFactory()
{
	m_factories.Add(this);
}

FontFactory::~FontFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
