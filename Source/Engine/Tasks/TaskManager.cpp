// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Engine/Tasks/TaskManager.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Thread.h"
#include "Generic/Math/Math.h"

#include "Generic/Threads/Atomic.h"

#include "Generic/Helper/StringHelper.h"

#include "Engine/Profiling/ProfilingManager.h"

// Dumps out execution logs.
//#define OPT_DUMP_EXECUTION_LOGS

void TaskManager::QueuedTask::Reset()
{
	ID				= -1;
	Work			= NULL;
	TaskRemaining	= 0;
	Parent			= 0;
	Dependency		= -1;
}

void TaskManager::Worker_Thread_Entry_Point(Thread* self, void* meta)
{
	TaskManager* manager = (TaskManager*)meta;

	int id = -1;
	for (int i = 0; i < manager->m_worker_count; i++)
	{
		if (manager->m_worker_threads[i] == self)
		{
			id = i;
			break;
		}
	}

	while (!manager->m_workers_exiting)
	{
		// We have a timeout just to prevent deadlocks that can happen in really
		// wierd ways (TODO: Debug and solve these!)
		Run_Available_Task(manager, id, 1);
	}
}

void TaskManager::Run_Available_Task(TaskManager* manager, int worker_id, unsigned int timeout)
{
	QueuedTask* task = manager->Worker_Wait_For_Task(worker_id, timeout);
	if (task != NULL)
	{
		manager->Run_Task(task);
	}
}

void TaskManager::Run_Task(QueuedTask* task)
{
	PROFILE_SCOPE("Parallel Task");

#ifdef MASTER_BUILD
	try
	{
#endif
		task->Work->Run();
#ifdef MASTER_BUILD
	}
	catch (...)
	{
		DBG_ASSERT_STR(false, "Captured unhandled exception within worker thread '%s'.", task->Work->Get_Name().c_str());
	}
#endif
	Worker_Task_Completed(task);
}

void TaskManager::Run_Task(TaskID id)
{
	QueuedTask* task = Get_Task_By_ID(id);
	if (task != NULL)
	{
		Run_Task(task);
	}
}

TaskManager::QueuedTask* TaskManager::Get_Task_By_ID(TaskID id)
{
	if (id == -1)
		return NULL;

	for (int i = 0; i < m_task_count; i++)
	{
		if (m_tasks[i].ID == id)
		{
			return &m_tasks[i];
		}
	}
	return NULL;
}

TaskManager::TaskManager(int max_workers, int max_tasks)
{
	m_worker_threads		 = new Thread*[max_workers];
	m_tasks					 = new QueuedTask[max_tasks];
	m_worker_count			 = max_workers;
	m_task_count			 = max_tasks;
	m_posts_pending			 = 0;

	for (int i = 0; i < max_tasks; i++)
	{
		m_tasks[i] = QueuedTask();
	}

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i] = NULL;
	}

	m_main_thread			= Thread::Get_Current();

	m_workers_exiting		 = false;
	m_next_task_id			 = 0;

	m_worker_semaphore       = Semaphore::Create();
	m_worker_task_list_mutex = Mutex::Create();
	
	DBG_LOG("Max tasks: %i", max_tasks);
	DBG_LOG("Max task workers: %i", max_workers);
	
	for (int i = 0; i < m_task_count; i++)
	{
		m_tasks[i].Reset();
		m_tasks[i].Free = true;
	}
	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i] = Thread::Create(StringHelper::Format("Task Worker %i", i).c_str(), Worker_Thread_Entry_Point, this);

		int cores = m_worker_threads[i]->Get_Core_Count();		
		if (cores > 1)
		{
			int core = 1 + (i % (cores - 1));
			DBG_LOG("Starting task worker thread %i on core %i.", i, core);

		//	m_worker_threads[i]->Set_Affinity(1 << core);
		}
		else
		{
			DBG_LOG("Starting task worker thread %i on primary core.");
		}

		m_worker_threads[i]->Start();
	}
}

TaskManager::~TaskManager()
{	
	m_workers_exiting = true;

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_semaphore->Signal();
	}

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_threads[i]->Join();
		SAFE_DELETE(m_worker_threads[i]);
	}

	SAFE_DELETE(m_worker_threads);
	SAFE_DELETE(m_tasks);

	SAFE_DELETE(m_worker_semaphore);
	SAFE_DELETE(m_worker_task_list_mutex);
}

TaskManager::QueuedTask* TaskManager::Worker_Wait_For_Task(int worker_id, unsigned int timeout)
{
	QueuedTask* task = Get_Available_Task(worker_id);
	if (task != NULL)
	{
		return task;
	}

	if (timeout > 0)
	{
		if (m_worker_semaphore->Wait(timeout))
		{
			task = Get_Available_Task(worker_id);
		}
	}
	
	return task;
}

void TaskManager::Worker_Post_New_Task(QueuedTask* task)
{
	Atomic::Increment32(&m_posts_pending);
	{
		MutexLock lock(m_worker_task_list_mutex);
		m_task_queue.push_back(task);
	}
	Atomic::Decrement32(&m_posts_pending);

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_semaphore->Signal();
	}
}

void TaskManager::Worker_Task_Completed(QueuedTask* task)
{
	Atomic::Decrement32(&task->TaskRemaining);
	task->ID = -1;
	
	if (task->TaskRemaining <= 0)
	{
		task->Work->Complete();
	}
	
	QueuedTask* parent = Get_Task_By_ID(task->Parent);
	if (parent != NULL)
	{
		Atomic::Decrement32(&parent->TaskRemaining);
	}
	
	if (task->TaskRemaining <= 0)
	{
		task->Free = true;
	}

	for (int i = 0; i < m_worker_count; i++)
	{
		m_worker_semaphore->Signal();
	}
} 

bool TaskManager::Is_Parent_Queued(QueuedTask* task)
{
	if (task->Parent == -1)
		return true;

	for (std::vector<QueuedTask*>::iterator iter = m_task_queue.begin(); iter != m_task_queue.end(); iter++)
	{
		QueuedTask* iter_task = *iter;
		if (iter_task->ID == task->Parent)
			return true;
	}

	return false;
}

TaskManager::QueuedTask* TaskManager::Get_Available_Task(int worker_id)
{
	QueuedTask* task = NULL;

	if (m_workers_exiting == true)
		return task;

	if (m_posts_pending > 0)
		return NULL;

	{
		MutexLock lock(m_worker_task_list_mutex);

		for (std::vector<QueuedTask*>::iterator iter = m_task_queue.begin(); iter != m_task_queue.end(); iter++)
		{
			QueuedTask* iter_task = *iter;
			if (iter_task->TaskRemaining == 1)		
			{
				if (Get_Task_By_ID(iter_task->Dependency) == NULL)
				{
					if (Can_Be_Run(iter_task))
					{
						if (Is_Parent_Queued(iter_task))
						{
							if (worker_id == 0xFFFFFFFF || (iter_task->Affinity & (1 << worker_id)) != 0)
							{
								iter = m_task_queue.erase(iter);
								task = iter_task;
								break;
							}
							else
							{
#ifdef OPT_DUMP_EXECUTION_LOGS
								DBG_LOG("Ignoring task %i, affinity is %i", iter_task->ID, iter_task->Affinity);
#endif
							}
						}
						else
						{
#ifdef OPT_DUMP_EXECUTION_LOGS
							DBG_LOG("Ignoring task %i, parent not queued.", iter_task->ID);
#endif
						}
 					}
					else
					{
#ifdef OPT_DUMP_EXECUTION_LOGS
						DBG_LOG("Ignoring task %i, cannot be run.", iter_task->ID);
#endif
					}
				}
				else
				{
#ifdef OPT_DUMP_EXECUTION_LOGS
					DBG_LOG("Ignoring task %i, dependency not met.", iter_task->ID);
#endif
				}
			}
			else
			{
#ifdef OPT_DUMP_EXECUTION_LOGS
				DBG_LOG("Ignoring task %i, still has sub tasks remaining.", iter_task->ID);
#endif
			}
		}
	}

#ifdef OPT_DUMP_EXECUTION_LOGS
	if (task == NULL)
	{
		DBG_LOG("No Tasks Available (queue size is %i) :(", m_task_queue.size());
	}
	else
	{
		DBG_LOG("Returning task %i", task->ID);
	}
#endif

	return task;
}

bool TaskManager::Can_Be_Run(QueuedTask* task)
{
	if (task->Work->Can_Run_On_Main_Thread())
	{
		return true;
	}
	else
	{
		return m_main_thread != Thread::Get_Current();
	}
}

TaskID TaskManager::Add_Task(Task* work, TaskID parent, u32 affinity)
{
	QueuedTask* taskParent = Get_Task_By_ID(parent);
	if (taskParent != NULL)
	{
		taskParent->TaskRemaining++;
	}

	for (int i = 0; i < m_task_count; i++)
	{
		if (m_tasks[i].Free == true)
		{
			m_tasks[i].Reset();
			m_tasks[i].Free			 = false;
			m_tasks[i].ID			 = (m_next_task_id++) % m_task_count;
			m_tasks[i].Parent		 = parent;
			m_tasks[i].TaskRemaining = 1;
			m_tasks[i].Work			 = work;
			m_tasks[i].Queued		 = false;
			m_tasks[i].Affinity		 = affinity;

			work->Reset();

			return m_tasks[i].ID;
		}
	}

	DBG_ASSERT_STR(false, "Failed to add task into task queue, queue has overflowed!");
	return 0;
}

void TaskManager::Depends_On(TaskID work, TaskID on)
{
	QueuedTask* task = Get_Task_By_ID(work);
	if (task != NULL)
	{
		task->Dependency = on;
	}
}

void TaskManager::Wait_For(TaskID work)
{
	PROFILE_SCOPE("Spin Wait For Task");
	while (true)
	{
		// Task completed yet?
		QueuedTask* task = Get_Task_By_ID(work);
		if (task == NULL)
			return;

		// Run some tasks while we are waiting.	
		{
			//PROFILE_SCOPE("Running Any Available Tasks");
			Run_Available_Task(this, 0xFFFFFFFF, 0);
		}
	}
}

void TaskManager::Queue_Task(TaskID work)
{
	QueuedTask* task = Get_Task_By_ID(work);
	if (task != NULL)
	{
		task->Queued = true;
		Worker_Post_New_Task(task);
	}
	else
	{
		DBG_ASSERT_STR(false, "Attempt to queue invalid task!?");
	}
}

void TaskManager::Wait_For_All()
{
	PROFILE_SCOPE("Spin Wait For All Tasks");
	while (true)
	{
		// Task completed yet?
		bool found_active_task = false;
		for (int i = 0; i < m_task_count; i++)
		{
			if (m_tasks[i].Free == false && 
				m_tasks[i].TaskRemaining > 0 &&
				m_tasks[i].Queued == true)
			{
				found_active_task = true;
				break;
 			}
		}
		if (found_active_task == false)
		{
			break;
		}

		// Run some tasks while we are waiting.
		{
			//PROFILE_SCOPE("Running Any Available Tasks");
			Run_Available_Task(this, 0xFFFFFFFF, 0);
		}
	}
}

