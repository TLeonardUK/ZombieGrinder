// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_MAP_MAPLOADTASK_
#define _ENGINE_MAP_MAPLOADTASK_

#include "Generic/Types/HashTable.h"

#include "Engine/Tasks/Task.h"

class Map;
class Actor;
class Stream;
class MapFileHandle;

class MapLoadTask : public Task
{
	MEMORY_ALLOCATOR(MapLoadTask, "Scene");

private:
	bool m_success;
	bool m_cancelled;
	Map* m_map;
	MapFileHandle* m_map_file;

public:	
	MapLoadTask(Map* map, MapFileHandle* map_file);
	~MapLoadTask();

	bool Was_Success();
	void Cancel();

	void Run();

};

#endif

