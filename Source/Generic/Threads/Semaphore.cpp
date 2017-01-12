// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/Semaphore.h"

#ifdef PLATFORM_WIN32
#include "Generic/Threads/Win32/Win32_Semaphore.h"
#elif defined(PLATFORM_MACOS)
#include "Generic/Threads/MacOS/MacOS_Semaphore.h"
#elif defined(PLATFORM_LINUX)
#include "Generic/Threads/Linux/Linux_Semaphore.h"
#endif

Semaphore* Semaphore::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Semaphore();
#elif defined(PLATFORM_MACOS)
	return new MacOS_Semaphore();
#elif defined(PLATFORM_LINUX)
	return new Linux_Semaphore();
#else
	#error "Platform unsupported."
#endif
}

