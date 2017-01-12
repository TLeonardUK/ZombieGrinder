// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Linux/Linux_ConditionVariable.h"
#include "Generic/Threads/Linux/Linux_Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include <time.h>

Linux_ConditionVariable::Linux_ConditionVariable()
{
    int ret = pthread_cond_init(&m_handle, NULL);
	DBG_ASSERT(ret == 0);
}

Linux_ConditionVariable::~Linux_ConditionVariable()
{
    int ret = pthread_cond_destroy(&m_handle);
	DBG_ASSERT(ret == 0);
}

void Linux_ConditionVariable::Broadcast()
{
	pthread_cond_signal(&m_handle);
}

void Linux_ConditionVariable::Signal()
{
	pthread_cond_broadcast(&m_handle);
}

bool Linux_ConditionVariable::Wait(Mutex* mutex, unsigned int timeout)
{
	Linux_Mutex* win32mutex = static_cast<Linux_Mutex*>(mutex);

	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	t.tv_sec  += timeout / 1000;
	t.tv_nsec += (timeout % 1000) * 1000000;

	int ret = pthread_cond_timedwait(&m_handle, &win32mutex->m_mutex, &t);

	return (ret == 0);
}

#endif