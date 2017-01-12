// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TASKS_GROUP_TASK_
#define _ENGINE_TASKS_GROUP_TASK_

#include "Engine/Tasks/Task.h"

#include <string.h>

// This does exactly jack shit, it's used to group tasks together so we can
// do things like WaitFor a group of them etc.
class GroupTask : public Task
{
	MEMORY_ALLOCATOR(GroupTask, "Engine");

private:
	std::string m_name;

public:
	GroupTask(std::string name);
	void Run();

	virtual std::string Get_Name();

};

#endif

