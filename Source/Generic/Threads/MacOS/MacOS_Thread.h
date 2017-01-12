// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _GENERIC_MACOS_THREAD_
#define _GENERIC_MACOS_THREAD_

#include "Generic/Threads/Thread.h"

#include <vector>
#include <pthread.h>
#include <semaphore.h>

class Mutex;
class Semaphore;

class MacOS_Thread : public Thread
{
	MEMORY_ALLOCATOR(MacOS_Thread, "Platform");

private:
	friend class Thread;

	MacOS_Thread(pthread_t id);
	MacOS_Thread(const char* name, EntryPoint entry_point, void* ptr);

	static std::vector<MacOS_Thread*> g_macos_threads;
	static Mutex* g_macos_threads_mutex;

	static MacOS_Thread* Get_Current_Thread();

	static void* Start_Routine(void* arg);
	
	EntryPoint	m_entry_point;
	void*		m_entry_ptr;
	pthread_t	m_thread;
	bool		m_finished;
	Semaphore*	m_suspended_semaphore;

public:

	~MacOS_Thread();
	
	static void Setup_Main();

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