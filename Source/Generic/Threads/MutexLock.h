// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MUTEXLOCK_
#define _GENERIC_MUTEXLOCK_

#include "Generic/Threads/Mutex.h"

// Used for RAII based locking and unlocking, eg.
//
//	{
//		MutexLock myLock(mutex);
//		<derp>
//	}
//
class MutexLock
{
	MEMORY_ALLOCATOR(MutexLock, "Platform");

private:
	Mutex* m_mutex;

public:
	MutexLock(Mutex* mutex);
	~MutexLock();

};

#endif

