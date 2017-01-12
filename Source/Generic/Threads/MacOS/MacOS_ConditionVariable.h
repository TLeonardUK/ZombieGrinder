// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _GENERIC_MACOS_CONDITIONVARIABLE_
#define _GENERIC_MACOS_CONDITIONVARIABLE_

#include "Generic/Threads/ConditionVariable.h"

#include <pthread.h>

class MacOS_ConditionVariable : public ConditionVariable
{
	MEMORY_ALLOCATOR(MacOS_ConditionVariable, "Platform");

private:
	friend class ConditionVariable;
	MacOS_ConditionVariable();
		
	pthread_cond_t m_handle;

public:
	~MacOS_ConditionVariable();
	
	void Broadcast();
	void Signal();
	bool Wait(Mutex* mutex, unsigned int timeout);

};

#endif

#endif