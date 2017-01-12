// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/MapFileFactory.h"
#include "Engine/Scene/Map/MapFile.h"
#include "Engine/Scene/Map/MapFileHandle.h"

#include "Generic/Helper/StringHelper.h"

ThreadSafeHashTable<MapFileHandle*, int>	MapFileFactory::m_loaded_maps;

MapFileFactory::MapFileFactory()
{
}

MapFileFactory::~MapFileFactory()
{
}

MapFileHandle* MapFileFactory::Load(const char* url)
{
	int url_hash = StringHelper::Hash(url);

	// Texture already loaded?
	if (m_loaded_maps.Contains(url_hash))
	{
		DBG_LOG("Loaded map from cache: %s", url);
		return m_loaded_maps.Get(url_hash);
	}

	// Try and load texture!
	MapFile* map = Load_Without_Handle(url);
	if (map != NULL)
	{		
		MapFileHandle* handle = new MapFileHandle(url, map);

		m_loaded_maps.Set(url_hash, handle);
		
		DBG_LOG("Loaded map: %s", url);

		return handle;
	}
	else
	{
		DBG_LOG("Failed to load map: %s", url);
	}

	return NULL;
}

MapFileHandle* MapFileFactory::New(const char* name)
{
	DBG_LOG("Creating new map file '%s'.", name);
	MapFile* map = new MapFile();
	return new MapFileHandle(name, map);
}

MapFile* MapFileFactory::Load_Without_Handle(const char* url)
{
	return MapFile::Load(url);
}

void MapFileFactory::Dispose()
{
	for (ThreadSafeHashTable<MapFileHandle*, int>::Iterator iter = m_loaded_maps.Begin(); iter != m_loaded_maps.End(); iter++)
	{
		MapFileHandle* handle = *iter;
		SAFE_DELETE(handle);
	}
	m_loaded_maps.Clear();
}
