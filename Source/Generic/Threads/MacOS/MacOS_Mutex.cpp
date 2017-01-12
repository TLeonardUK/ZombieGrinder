// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Threads/MacOS/MacOS_Mutex.h"

MacOS_Mutex::MacOS_Mutex()
{
	pthread_mutexattr_init(&m_attr);
	pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);

    int ret = pthread_mutex_init(&m_mutex, &m_attr);
	DBG_ASSERT(ret == 0);
}

MacOS_Mutex::~MacOS_Mutex()
{
    int ret = pthread_mutex_destroy(&m_mutex);
	DBG_ASSERT(ret == 0);
}

bool MacOS_Mutex::Try_Lock()
{
	int ret = pthread_mutex_trylock(&m_mutex);
	return (ret == 0);
}

void MacOS_Mutex::Lock()
{
	int ret = pthread_mutex_lock(&m_mutex);
	DBG_ASSERT(ret == 0);
}

void MacOS_Mutex::Unlock()
{
	int ret = pthread_mutex_unlock(&m_mutex);
	DBG_ASSERT(ret == 0);
}

#endif