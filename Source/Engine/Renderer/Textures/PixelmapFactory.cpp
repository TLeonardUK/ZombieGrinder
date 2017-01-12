// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Renderer/Textures/PixelmapFactory.h"
#include "Engine/Renderer/Textures/Pixelmap.h"

#include "Generic/Helper/StringHelper.h"

LinkedList<PixelmapFactory*> PixelmapFactory::m_factories;

bool PixelmapFactory::Save(const char* url, Pixelmap* texture)
{
	for (LinkedList<PixelmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixelmapFactory* factory = *iter;
		bool result = factory->Try_Save(url, texture);
		if (result != false)
		{
			//DBG_LOG("Saved Pixelmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to save Pixelmap: %s", url);
	return NULL;
}

Pixelmap* PixelmapFactory::Load(const char* url)
{
	for (LinkedList<PixelmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixelmapFactory* factory = *iter;
		Pixelmap* result = factory->Try_Load(url);
		if (result != NULL)
		{
			//DBG_LOG("Loaded Pixelmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load Pixelmap: %s", url);
	return NULL;
}

bool PixelmapFactory::Save(Stream* url, Pixelmap* texture)
{
	for (LinkedList<PixelmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixelmapFactory* factory = *iter;
		bool result = factory->Try_Save(url, texture);
		if (result != false)
		{
			//DBG_LOG("Saved Pixelmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to save Pixelmap: %s", url);
	return NULL;
}

Pixelmap* PixelmapFactory::Load(Stream* url)
{
	for (LinkedList<PixelmapFactory*>::Iterator iter = m_factories.Begin(); iter != m_factories.End(); iter++)
	{
		PixelmapFactory* factory = *iter;
		Pixelmap* result = factory->Try_Load(url);
		if (result != NULL)
		{
			//DBG_LOG("Loaded Pixelmap: %s", url);
			return result;
		}
	}

	DBG_LOG("Failed to load Pixelmap: %s", url);
	return NULL;
}

PixelmapFactory::PixelmapFactory()
{
	m_factories.Add(this);
}

PixelmapFactory::~PixelmapFactory()
{
	m_factories.Remove(m_factories.Find(this));
}
