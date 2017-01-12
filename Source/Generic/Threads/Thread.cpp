// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Threads/Thread.h"

#ifdef PLATFORM_WIN32
#include "Generic/Threads/Win32/Win32_Thread.h"
#elif defined(PLATFORM_MACOS)
#include "Generic/Threads/MacOS/MacOS_Thread.h"
#elif defined(PLATFORM_LINUX)
#include "Generic/Threads/Linux/Linux_Thread.h"
#endif

Thread::Thread(const char* name)
{
	m_name = (name == NULL ? "<unnamed>" : name);
}

Thread* Thread::Create(const char* name, EntryPoint entry_point, void* ptr)
{
#ifdef PLATFORM_WIN32
	return new Win32_Thread(name, entry_point, ptr);
#elif defined(PLATFORM_MACOS)
	return new MacOS_Thread(name, entry_point, ptr);
#elif defined(PLATFORM_LINUX)
	return new Linux_Thread(name, entry_point, ptr);
#else
	#error "Platform unsupported."
#endif
}

Thread* Thread::Get_Current()
{
#ifdef PLATFORM_WIN32
	return Win32_Thread::Get_Current_Thread();
#elif defined(PLATFORM_MACOS)
	return MacOS_Thread::Get_Current_Thread();
#elif defined(PLATFORM_LINUX)
	return Linux_Thread::Get_Current_Thread();
#else
	#error "Platform unsupported."
#endif
}