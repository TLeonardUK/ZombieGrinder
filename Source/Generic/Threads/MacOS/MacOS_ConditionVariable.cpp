// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Threads/MacOS/MacOS_ConditionVariable.h"
#include "Generic/Threads/MacOS/MacOS_Mutex.h"
#include "Generic/Threads/MutexLock.h"

MacOS_ConditionVariable::MacOS_ConditionVariable()
{
    int ret = pthread_cond_init(&m_handle, NULL);
	DBG_ASSERT(ret == 0);
}

MacOS_ConditionVariable::~MacOS_ConditionVariable()
{
    int ret = pthread_cond_destroy(&m_handle);
	DBG_ASSERT(ret == 0);
}

void MacOS_ConditionVariable::Broadcast()
{
	pthread_cond_signal(&m_handle);
}

void MacOS_ConditionVariable::Signal()
{
	pthread_cond_broadcast(&m_handle);
}

bool MacOS_ConditionVariable::Wait(Mutex* mutex, unsigned int timeout)
{
	MacOS_Mutex* win32mutex = static_cast<MacOS_Mutex*>(mutex);

	struct timespec t;
	t.tv_sec  = timeout / 1000;
	t.tv_nsec = (timeout % 1000) * 1000000;

	int ret = 0;

	if (timeout == 0)
	{
		ret = pthread_cond_wait(&m_handle, &win32mutex->m_mutex);
	}
	else
	{
		ret = pthread_cond_timedwait(&m_handle, &win32mutex->m_mutex, &t);
	}

	return (ret == 0);
}

#endif