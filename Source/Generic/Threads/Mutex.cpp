// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/Mutex.h"

#ifdef PLATFORM_WIN32
#include "Generic/Threads/Win32/Win32_Mutex.h"
#elif defined(PLATFORM_MACOS)
#include "Generic/Threads/MacOS/MacOS_Mutex.h"
#elif defined(PLATFORM_LINUX)
#include "Generic/Threads/Linux/Linux_Mutex.h"
#endif

Mutex* Mutex::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_Mutex();
#elif defined(PLATFORM_MACOS)
	return new MacOS_Mutex();
#elif defined(PLATFORM_LINUX)
	return new Linux_Mutex();
#else
	#error "Platform unsupported."
#endif
}

