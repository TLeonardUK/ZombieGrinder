// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _GENERIC_LINUX_MUTEX_
#define _GENERIC_LINUX_MUTEX_

#include "Generic/Threads/Mutex.h"

#include <pthread.h>

class Linux_ConditionVariable;

class Linux_Mutex : public Mutex
{
	MEMORY_ALLOCATOR(Linux_Mutex, "Platform");

private:
	friend class Mutex;
	friend class Linux_ConditionVariable;

	pthread_mutex_t m_mutex;
	pthread_mutexattr_t m_attr;

	Linux_Mutex();

public:

	~Linux_Mutex();
	
	bool Try_Lock	();
	void Lock		();
	void Unlock		();

};

#endif

#endif