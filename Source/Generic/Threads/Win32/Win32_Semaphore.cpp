// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Win32/Win32_Semaphore.h"

Win32_Semaphore::Win32_Semaphore()
{
	m_handle = CreateSemaphore(NULL, 0, 1000000, NULL);
	DBG_ASSERT(m_handle != NULL);
}

Win32_Semaphore::~Win32_Semaphore()
{
	CloseHandle(m_handle);
}

void Win32_Semaphore::Signal()
{
	ReleaseSemaphore(m_handle, 1, NULL);
}

bool Win32_Semaphore::Wait(int timeout)
{
	if (timeout == 0)
	{
		timeout = INFINITE;
	}
	return (WaitForSingleObject(m_handle, timeout) != 0);
}

#endif