// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPCACHE_
#define _ENGINE_SCENE_MAP_MAPCACHE_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/HashTable.h"
#include "Generic/Patterns/Singleton.h"

class MapFile;
class MapFileHandle;

// The map cache stores all currently loaded maps, 

class MapCache : public Singleton<MapCache>
{
	MEMORY_ALLOCATOR(MapCache, "Scene");

public:
	MapCache();
	~MapCache();


};

#endif

