// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TASKS_TASK_
#define _ENGINE_TASKS_TASK_

#include <string>

// Used in conjunction with the task manager to spin up and run
// a multi-threaded task.
class Task
{
	MEMORY_ALLOCATOR(Task, "Engine");

private:
	bool m_completed;

protected:
	friend class TaskManager;

	Task();
	void Reset();
	void Complete();

public:	
	bool Is_Completed();
	
	virtual std::string Get_Name()
	{
		return "Untitled Task";
	}

	virtual bool Can_Run_On_Main_Thread()
	{
		return true;
	}

	virtual void Run() = 0;

};

#endif

