// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MAP_MAPSERIALIZER_
#define _ENGINE_MAP_MAPSERIALIZER_

#include "Generic/Types/HashTable.h"

#include "Engine/Tasks/Task.h"

class Map;
class Actor;
class Stream;
class MapFileHandle;

class MapSerializer 
{
	MEMORY_ALLOCATOR(MapSerializer, "Scene");

private:
	Map* m_map;

public:	
	MapSerializer(Map* map);

	void Serialize(Stream* stream);
	void Serialize(const char* path);

};

#endif

