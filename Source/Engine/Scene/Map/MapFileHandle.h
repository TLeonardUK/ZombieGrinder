// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILEHANDLE_
#define _ENGINE_SCENE_MAP_MAPFILEHANDLE_

#include "Engine/Engine/FrameTime.h"

#include "Engine/Resources/Reloadable.h"

#include <string>

class MapFileFactory;
class MapFile;

class MapFileHandle : public Reloadable
{
	MEMORY_ALLOCATOR(MapFileHandle, "Scene");

private:

	MapFile*			m_map;
	std::string			m_url;

protected:
	
	friend class MapFileFactory;

	// Only texture factory should be able to modify these!
	MapFileHandle(const char* url, MapFile* layout);
	~MapFileHandle();
	
	void Reload();

public:

	// Get/Set
	MapFile* Get();
	std::string Get_URL();

};

#endif

