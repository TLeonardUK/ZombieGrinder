// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/ConditionVariable.h"

#ifdef PLATFORM_WIN32
#include "Generic/Threads/Win32/Win32_ConditionVariable.h"
#elif defined(PLATFORM_MACOS)
#include "Generic/Threads/MacOS/MacOS_ConditionVariable.h"
#elif defined(PLATFORM_LINUX)
#include "Generic/Threads/Linux/Linux_ConditionVariable.h"
#endif

ConditionVariable* ConditionVariable::Create()
{
#ifdef PLATFORM_WIN32
	return new Win32_ConditionVariable();
#elif defined(PLATFORM_MACOS)
	return new MacOS_ConditionVariable();
#elif defined(PLATFORM_LINUX)
	return new Linux_ConditionVariable();
#else
	#error "Platform unsupported."
#endif
}

