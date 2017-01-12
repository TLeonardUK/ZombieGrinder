// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MUTEX_
#define _GENERIC_MUTEX_

// Note on implementation:
//
//		Do not assume a mutex can or cannot recursively lock, this differs
//		by platform. Make sure you don't design functionality based around recursive
//		locking.

class Mutex
{
	MEMORY_ALLOCATOR(Mutex, "Platform");

public:
	static Mutex* Create();

	// Lock / Unlock shenanigans.
	virtual bool Try_Lock	() = 0;
	virtual void Lock		() = 0;
	virtual void Unlock		() = 0;

};

#endif

