// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Mutex.h"

MutexLock::MutexLock(Mutex* mutex)
{
	m_mutex = mutex;
	if (mutex != NULL)
	{
		m_mutex->Lock();
	}
}

MutexLock::~MutexLock()
{
	if (m_mutex != NULL)
	{
		m_mutex->Unlock();
		m_mutex = NULL;
	}
}
