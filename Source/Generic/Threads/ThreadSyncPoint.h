// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_THREADSYNCPOINT_
#define _GENERIC_THREADSYNCPOINT_

// Allows you to sync multiple threads at a given point.
//
// Used like so;
//
//	sync->Start(total_workers);
//  if (sync->Wait())
//	{
//		sync->Continue();
//	}

class Semaphore;

class ThreadSyncPoint
{
	MEMORY_ALLOCATOR(ThreadSyncPoint, "Platform");

protected:
	Semaphore* m_semaphore;
	int m_remaining;
	int m_initial_workers;

public:	
	ThreadSyncPoint();
	~ThreadSyncPoint();

	void Start(int workers);
	bool Wait();
	void Continue();

};

#endif

