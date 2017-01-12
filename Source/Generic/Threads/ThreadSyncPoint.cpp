// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/ThreadSyncPoint.h"
#include "Generic/Threads/Atomic.h"
#include "Generic/Threads/Semaphore.h"

ThreadSyncPoint::ThreadSyncPoint()
{
	m_semaphore = Semaphore::Create();
}

ThreadSyncPoint::~ThreadSyncPoint()
{
	SAFE_DELETE(m_semaphore);
}

void ThreadSyncPoint::Start(int workers)
{
	m_remaining = workers;
	m_initial_workers = workers;
}

bool ThreadSyncPoint::Wait()
{
	// Last to finish - return control.
	if (Atomic::Decrement32(&m_remaining) == 0)
	{
		return true;
	}

	// Otherwise block until we are done.
	{
		m_semaphore->Wait();
	}

	return false;
}

void ThreadSyncPoint::Continue()
{
	for (int i = 0; i < m_initial_workers - 1; i++)
	{
		m_semaphore->Signal();
	}
}