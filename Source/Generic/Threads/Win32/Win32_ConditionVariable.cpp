// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Win32/Win32_ConditionVariable.h"
#include "Generic/Threads/Win32/Win32_Mutex.h"
#include "Generic/Threads/MutexLock.h"

Win32_ConditionVariable::Win32_ConditionVariable()
{
//	InitializeConditionVariable(&m_convar_handle);	
	m_handle = CreateSemaphore(NULL, 0, 1000000, NULL);
	DBG_ASSERT(m_handle != NULL);
}

Win32_ConditionVariable::~Win32_ConditionVariable()
{
	CloseHandle(m_handle);
}

void Win32_ConditionVariable::Broadcast()
{
	ReleaseSemaphore(m_handle, 1, NULL);
	//WakeAllConditionVariable(&m_convar_handle);
}

void Win32_ConditionVariable::Signal()
{
	DBG_ASSERT_STR(false, "No longer supported - Not available on windows-xp!");
	//ReleaseSemaphore(m_handle, 1, NULL);
	//WakeConditionVariable(&m_convar_handle);
}

bool Win32_ConditionVariable::Wait(Mutex* mutex, unsigned int timeout)
{
	Win32_Mutex* win32mutex = static_cast<Win32_Mutex*>(mutex);

	if (timeout == 0)
	{
		timeout = INFINITE;
	}

	// Not really 100% safe, but best we can do without a proper OS implementation of condvar's
	win32mutex->Unlock();	
	return (WaitForSingleObject(m_handle, timeout) != 0);
	//return (SleepConditionVariableCS(&m_convar_handle, &win32mutex->m_critical_section, timeout) != 0);
}

#endif