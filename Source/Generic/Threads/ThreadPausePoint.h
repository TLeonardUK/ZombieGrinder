// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREADPAUSEPOINT_
#define _GENERIC_THREADPAUSEPOINT_

// Allows you to pause and resume multiple threads. Most handy
// when you need to stop multiple worker threads for a eriod
// while you do something not thread safe.
//
//	Pause Thread:
//		sync.Pause();
//		blah blah blah blah blah
//		sync.Resume();
//
//	Worker Thread:
//		sync.Poll();
//

class Semaphore;

class ThreadPausePoint
{
	MEMORY_ALLOCATOR(ThreadPausePoint, "Platform");

protected:
	Semaphore* m_pause_semaphore;
	Semaphore* m_resume_semaphore;

	int m_paused_workers;
	int m_active_workers;
	bool m_paused;

public:	
	ThreadPausePoint();
	~ThreadPausePoint();

	void Pause(int workers, Semaphore* wake_semaphore = NULL);
	void Resume();
	void Poll();

};

#endif

