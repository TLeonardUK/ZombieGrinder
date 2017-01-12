// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _GENERIC_LINUX_THREAD_
#define _GENERIC_LINUX_THREAD_

#include "Generic/Threads/Thread.h"

#include <vector>
#include <pthread.h>
#include <semaphore.h>

class Mutex;
class Semaphore;

class Linux_Thread : public Thread
{
	MEMORY_ALLOCATOR(Linux_Thread, "Platform");

private:
	friend class Thread;

	Linux_Thread(pthread_t id);
	Linux_Thread(const char* name, EntryPoint entry_point, void* ptr);

	static std::vector<Linux_Thread*> g_linux_threads;
	static Mutex* g_linux_threads_mutex;

	static Linux_Thread* Get_Current_Thread();

	static void* Start_Routine(void* arg);
	
	EntryPoint	m_entry_point;
	void*		m_entry_ptr;
	pthread_t	m_thread;
	bool		m_finished;
	Semaphore*	m_suspended_semaphore;

public:
	
	static void Setup_Main();

	~Linux_Thread();

	// Control functions.
	void Start();
	void Set_Priority(ThreadPriority::Type priority);
	void Set_Affinity(int mask);

	// General functions.
	bool Is_Running();
	void Join();

	// Functions that should be static! TODO
	void Sleep(float seconds);
	int  Get_Core_Count();
	void Yield();

};

#endif

#endif