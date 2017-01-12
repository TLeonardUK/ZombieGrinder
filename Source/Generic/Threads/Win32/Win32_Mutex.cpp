// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Win32/Win32_Mutex.h"

Win32_Mutex::Win32_Mutex()
{
	InitializeCriticalSection(&m_critical_section);
}

Win32_Mutex::~Win32_Mutex()
{
	DeleteCriticalSection(&m_critical_section);
}

bool Win32_Mutex::Try_Lock()
{
	return TryEnterCriticalSection(&m_critical_section) != 0;
}

void Win32_Mutex::Lock()
{
	EnterCriticalSection(&m_critical_section);
}

void Win32_Mutex::Unlock()
{
	LeaveCriticalSection(&m_critical_section);
}

#endif