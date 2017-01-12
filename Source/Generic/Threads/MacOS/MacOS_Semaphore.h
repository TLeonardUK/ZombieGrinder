// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _GENERIC_MACOS_SEMAPHORE_
#define _GENERIC_MACOS_SEMAPHORE_

#include "Generic/Threads/Semaphore.h"

#include <dispatch/dispatch.h>

class ConditionVariable;
class Mutex;

class MacOS_Semaphore : public Semaphore
{
	MEMORY_ALLOCATOR(MacOS_Semaphore, "Platform");

private:
	friend class Semaphore;
	MacOS_Semaphore();

	dispatch_semaphore_t m_semaphore;

//	int m_counter;

//	ConditionVariable* m_cond_var;
//	Mutex* m_mutex;

public:

	~MacOS_Semaphore();

	void Signal	();
	bool Wait	(int timeout);

};

#endif

#endif