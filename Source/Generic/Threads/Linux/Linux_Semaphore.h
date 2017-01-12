// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _GENERIC_LINUX_SEMAPHORE_
#define _GENERIC_LINUX_SEMAPHORE_

#include "Generic/Threads/Semaphore.h"

#include <semaphore.h>

class Linux_Semaphore : public Semaphore
{
	MEMORY_ALLOCATOR(Linux_Semaphore, "Platform");

private:
	friend class Semaphore;
	Linux_Semaphore();

	sem_t m_semaphore;

public:

	~Linux_Semaphore();

	void Signal	();
	bool Wait	(int timeout);

};

#endif

#endif