// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_SEMAPHORE_
#define _GENERIC_SEMAPHORE_

class Semaphore
{
	MEMORY_ALLOCATOR(Semaphore, "Platform");

public:
	static Semaphore* Create();

	// Syncronization.
	virtual void Signal	() = 0;
	virtual bool Wait	(int timeout = 0) = 0;

};

#endif

