// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Linux/Linux_Semaphore.h"

#include <errno.h>
#include <time.h>

Linux_Semaphore::Linux_Semaphore()
{
	int ret = sem_init(&m_semaphore, 0, 0);
	DBG_ASSERT_STR(ret != -1, "sem_init returned errno %i", errno);
}

Linux_Semaphore::~Linux_Semaphore()
{
	int ret = sem_destroy(&m_semaphore);
	DBG_ASSERT_STR(ret == 0, "sem_destroy returned errno %i", errno);
}

void Linux_Semaphore::Signal()
{
	int ret = sem_post(&m_semaphore);
	DBG_ASSERT_STR(ret == 0, "sem_post returned errno %i", errno);
}

bool Linux_Semaphore::Wait(int timeout)
{
	if (timeout == 0)
	{
		int ret = sem_wait(&m_semaphore);
		return (ret == 0);
	}
	else
	{
		struct timespec t;
		clock_gettime(CLOCK_REALTIME, &t);
		t.tv_sec  += timeout / 1000;
		t.tv_nsec += (timeout % 1000) * 1000000;

		int ret = sem_timedwait(&m_semaphore, &t);

		return (ret == 0);
	}
}

#endif