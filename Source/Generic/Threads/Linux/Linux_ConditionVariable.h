// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _GENERIC_LINUX_CONDITIONVARIABLE_
#define _GENERIC_LINUX_CONDITIONVARIABLE_

#include "Generic/Threads/ConditionVariable.h"

#include <pthread.h>

class Linux_ConditionVariable : public ConditionVariable
{
	MEMORY_ALLOCATOR(Linux_ConditionVariable, "Platform");

private:
	friend class ConditionVariable;
	Linux_ConditionVariable();
		
	pthread_cond_t m_handle;

public:
	~Linux_ConditionVariable();
	
	void Broadcast();
	void Signal();
	bool Wait(Mutex* mutex, unsigned int timeout);

};

#endif

#endif