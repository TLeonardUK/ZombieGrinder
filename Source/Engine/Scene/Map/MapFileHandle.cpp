// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Scene/Map/MapFileFactory.h"
#include "Engine/Scene/Map/MapFileHandle.h"
#include "Engine/Scene/Map/MapFile.h"

MapFileHandle::MapFileHandle(const char* url, MapFile* layout)
	: m_url(url)
	, m_map(layout)
{
	Add_Reload_Trigger_File(url);
}

MapFileHandle::~MapFileHandle()
{
	SAFE_DELETE(m_map);
}
	
void MapFileHandle::Reload()
{
	MapFile* new_layout = MapFileFactory::Load_Without_Handle(m_url.c_str());
	if (new_layout != NULL)
	{
		SAFE_DELETE(m_map);			
		m_map = new_layout;

		DBG_LOG("Reloaded changed map: %s", m_url.c_str());
	}
}

MapFile* MapFileHandle::Get()
{
	return m_map;
}

std::string MapFileHandle::Get_URL()
{
	return m_url;
}