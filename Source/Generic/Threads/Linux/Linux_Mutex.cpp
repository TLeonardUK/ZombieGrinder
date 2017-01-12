// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Linux/Linux_Mutex.h"

#include <cstring>

Linux_Mutex::Linux_Mutex()
{
	pthread_mutexattr_init(&m_attr);
	pthread_mutexattr_settype(&m_attr, PTHREAD_MUTEX_RECURSIVE);

	int ret = pthread_mutex_init(&m_mutex, &m_attr);
	DBG_ASSERT(ret == 0);
}

Linux_Mutex::~Linux_Mutex()
{
    int ret = pthread_mutex_destroy(&m_mutex);
	DBG_ASSERT(ret == 0);
}

bool Linux_Mutex::Try_Lock()
{
	int ret = pthread_mutex_trylock(&m_mutex);
	return (ret == 0);
}

void Linux_Mutex::Lock()
{
	int ret = pthread_mutex_lock(&m_mutex);
	DBG_ASSERT(ret == 0);
}

void Linux_Mutex::Unlock()
{
	int ret = pthread_mutex_unlock(&m_mutex);
	DBG_ASSERT(ret == 0);
}

#endif