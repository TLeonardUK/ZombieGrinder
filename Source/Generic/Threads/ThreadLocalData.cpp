// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/ThreadLocalData.h"

#ifdef PLATFORM_WIN32
#include "Generic/Threads/Win32/Win32_ThreadLocalData.h"
#elif defined(PLATFORM_MACOS)
#include "Generic/Threads/MacOS/MacOS_ThreadLocalData.h"
#elif defined(PLATFORM_LINUX)
#include "Generic/Threads/Linux/Linux_ThreadLocalData.h"
#endif

Platform_ThreadLocalData* Platform_ThreadLocalData::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_ThreadLocalData();
#elif defined(PLATFORM_MACOS)
	return new MacOS_ThreadLocalData();
#elif defined(PLATFORM_LINUX)
	return new Linux_ThreadLocalData();
#else
	#error "Platform unsupported."
#endif
}

