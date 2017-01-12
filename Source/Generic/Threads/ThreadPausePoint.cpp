// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/ThreadPausePoint.h"
#include "Generic/Threads/Atomic.h"
#include "Generic/Threads/Semaphore.h"

ThreadPausePoint::ThreadPausePoint()
	: m_paused(false)
	, m_paused_workers(0)
	, m_active_workers(0)
{
	m_pause_semaphore = Semaphore::Create();
	m_resume_semaphore = Semaphore::Create();
}

ThreadPausePoint::~ThreadPausePoint()
{
	SAFE_DELETE(m_pause_semaphore);
	SAFE_DELETE(m_resume_semaphore);
}

void ThreadPausePoint::Pause(int workers, Semaphore* wake_semaphore)
{
	DBG_ASSERT(m_paused == false);

	m_paused_workers = 0;
	m_active_workers = workers;
	m_paused = true;

	while (m_paused_workers < m_active_workers)
	{
		wake_semaphore->Signal();
		m_pause_semaphore->Wait();
	}
}

void ThreadPausePoint::Resume()
{
	DBG_ASSERT(m_paused == true);

	m_paused = false;

	while (m_paused_workers > 0)
	{
		m_resume_semaphore->Signal();
		m_pause_semaphore->Wait();
	}
}

void ThreadPausePoint::Poll()
{
	if (m_paused == true)
	{
		Atomic::Increment32(&m_paused_workers);
		m_pause_semaphore->Signal();
		m_resume_semaphore->Wait();
		Atomic::Decrement32(&m_paused_workers);
		m_pause_semaphore->Signal();
	}
}