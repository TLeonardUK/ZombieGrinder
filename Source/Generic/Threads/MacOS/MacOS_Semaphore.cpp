// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Threads/MacOS/MacOS_Semaphore.h"
#include "Generic/Threads/ConditionVariable.h"
#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"
#include "Generic/Threads/Atomic.h"

#include "Generic/Helper/StringHelper.h"

#include <unistd.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <mach/task.h>
#include <mach/semaphore.h>
#include <pthread.h>
#include <sched.h>
#include <dispatch/dispatch.h>

// Note: This is all wrong, fix.

MacOS_Semaphore::MacOS_Semaphore()
{
	m_semaphore = dispatch_semaphore_create(0);
}

MacOS_Semaphore::~MacOS_Semaphore()
{
	dispatch_release(m_semaphore);
}

void MacOS_Semaphore::Signal()
{
	dispatch_semaphore_signal(m_semaphore);
}

bool MacOS_Semaphore::Wait(int timeout)
{
	if (timeout == 0)
	{
		dispatch_semaphore_wait(m_semaphore, DISPATCH_TIME_FOREVER);
		return true;
	}
	else
	{
		long retval = dispatch_semaphore_wait(m_semaphore, dispatch_time(DISPATCH_TIME_NOW, timeout * 1000000));
		return (retval == 0);
	}
}

#endif