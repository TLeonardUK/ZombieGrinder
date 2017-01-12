// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_RESOURCES_RESOURCE_LOAD_TASK_
#define _ENGINE_RESOURCES_RESOURCE_LOAD_TASK_

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/Tasks/Task.h"


// Used by ResourceFactory to load a single resource file.
class ResourceLoadTask : public Task
{
	MEMORY_ALLOCATOR(ResourceLoadTask, "Engine");

private:
	ResourceFactory* m_factory;
	PackageFile* m_package_file;
	PackageFileChunk* m_package_chunk;

public:
	ResourceLoadTask(ResourceFactory* factory, PackageFile* file, PackageFileChunk* chunk);

	bool Can_Run_On_Main_Thread()
	{
		return false;
	}

	void Run();

};

#endif

