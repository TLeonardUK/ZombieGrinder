// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_SCENE_MAP_MAPFILEFACTORY_
#define _ENGINE_SCENE_MAP_MAPFILEFACTORY_

#include "Engine/Engine/FrameTime.h"
#include "Generic/Types/ThreadSafeHashTable.h"

class MapFile;
class MapFileHandle;

class MapFileFactory
{
	MEMORY_ALLOCATOR(MapFileFactory, "Scene");

protected:
	static ThreadSafeHashTable<MapFileHandle*, int>	m_loaded_maps;

	MapFileFactory();
	~MapFileFactory();

public:

	// Disposal.
	static void Dispose();
		
	// Static methods.
	static MapFileHandle*	Load				  (const char* url);
	static MapFile*			Load_Without_Handle	  (const char* url);

	static MapFileHandle*	New					  (const char* name);

};

#endif

