// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _GENERIC_MACOS_MUTEX_
#define _GENERIC_MACOS_MUTEX_

#include "Generic/Threads/Mutex.h"

#include <pthread.h>

class MacOS_ConditionVariable;

class MacOS_Mutex : public Mutex
{
	MEMORY_ALLOCATOR(MacOS_Mutex, "Platform");

private:
	friend class Mutex;
	friend class MacOS_ConditionVariable;

	pthread_mutex_t m_mutex;
	pthread_mutexattr_t m_attr;

	MacOS_Mutex();

public:

	~MacOS_Mutex();
	
	bool Try_Lock	();
	void Lock		();
	void Unlock		();

};

#endif

#endif