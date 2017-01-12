// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_TASKS_TASKMANAGER_
#define _ENGINE_TASKS_TASKMANAGER_

#include "Engine/Tasks/Task.h"
#include "Generic/Patterns/Singleton.h"

#include <vector>
#include "Generic/Threads/Thread.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/ConditionVariable.h"
#include "Generic/Threads/Semaphore.h"

// A task ID is used to reference a task once it's been queued with the
// task amanager. It pretty much just equates to an index in the task array.
typedef int TaskID;

class TaskManager : public Singleton<TaskManager>
{
	MEMORY_ALLOCATOR(TaskManager, "Engine");

private:
	struct QueuedTask
	{
		TaskID			ID;
		Task*			Work;
		int				TaskRemaining;
		TaskID			Parent;
		TaskID			Dependency;	
		bool			Free;
		bool			Queued;
		u32				Affinity;

		void Reset();
	};

private:
	// Worker information variables.
	TaskID		 m_next_task_id;
	QueuedTask*  m_tasks;
	int			 m_task_count;

	std::vector<QueuedTask*> m_task_queue;

	//ConditionVariable*	m_worker_task_con_var;
	Semaphore*			m_worker_semaphore;
	Mutex*				m_worker_task_list_mutex;
	//Mutex*				m_worker_task_mutex;
	Thread**			m_worker_threads;
	int					m_worker_count;

	bool				m_workers_exiting;

	Thread*				m_main_thread;

	int					m_posts_pending;

	// Entry point for work threads.
	static void Worker_Thread_Entry_Point(Thread* self, void* meta);
	static void Run_Available_Task(TaskManager* manager, int worker_id, unsigned int timeout);

	// Used internally by workers to syncronize themselves.
	QueuedTask*	Worker_Wait_For_Task	(int worker_id, unsigned int timeout = 0);
	void		Worker_Post_New_Task	(QueuedTask* task);
	void		Worker_Task_Completed	(QueuedTask* task);

	QueuedTask*	Get_Available_Task		(int worker_id);
	QueuedTask*	Get_Task_By_ID			(TaskID id);
	void		Run_Task				(QueuedTask* task);
	bool		Is_Parent_Queued		(QueuedTask* task);
	bool		Can_Be_Run				(QueuedTask* task);

public:

	// Constructor/Destructor stuff.
	TaskManager							(int max_workers, int max_tasks);
	~TaskManager						();

	// General.
	INLINE int Get_Worker_Count() { return m_worker_count; }

	// Task management.
	TaskID Add_Task						(Task* work, TaskID parent = -1, u32 affinity = 0xFFFFFFFF);
	void   Depends_On					(TaskID work, TaskID on);
	void   Wait_For						(TaskID work);
	void   Queue_Task					(TaskID work);
	void   Wait_For_All					();

	// Used to run tasks blocking. Don't call for Queued tasks as you don't
	// know if they are currently running on another thread!
	void   Run_Task						(TaskID task);

	// Public call to Run_Available_task.
	void   Run_Any_Task(unsigned int timeout)
	{
		Run_Available_Task(this, 0xFFFFFFFF, timeout);
	}

};

#endif

