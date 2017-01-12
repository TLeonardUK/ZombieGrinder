// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Win32/Win32_Thread.h"
#include "Generic/Threads/Win32/Win32_Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include "Engine/Profiling/ProfilingManager.h"

__declspec(thread) Win32_Thread* tls_self = NULL;

// This lovely struct + function are used to set the name of a thread
// in the visual studio debugger. Only works in visual studio!
typedef struct tagTHREADNAME_INFO
{
   DWORD dwType; // must be 0x1000
   LPCSTR szName; // pointer to name (in user addr space)
   DWORD dwThreadID; // thread ID (-1=caller thread)
   DWORD dwFlags; // reserved for future use, must be zero
} THREADNAME_INFO;

void Set_Visual_Studio_Thread_Name(DWORD dwThreadID, const char* szThreadName)
{
	if (IsDebuggerPresent())
	{
		THREADNAME_INFO info;
		info.dwType			= 0x1000;
		info.szName			= szThreadName;
		info.dwThreadID		= dwThreadID;
		info.dwFlags		= 0;

		__try
		{
			RaiseException(0x406D1388, 0, sizeof(info)/sizeof(DWORD), (DWORD*)&info);
		}
		__except(EXCEPTION_CONTINUE_EXECUTION)
		{
		}
	}
}

void Win32_Thread::Setup_Main()
{
	DWORD id = GetCurrentThreadId();
	
	HANDLE handle;
	DuplicateHandle(GetCurrentProcess(),
					GetCurrentThread(), 
					GetCurrentProcess(),
					&handle,       
					0,                  
					TRUE,               
					DUPLICATE_SAME_ACCESS);

	tls_self = new Win32_Thread(handle, id);
	tls_self->Set_Name("Main Thread");
}

Win32_Thread* Win32_Thread::Get_Current_Thread()
{
	return tls_self;

	/*
	MutexLock lock(g_win32_threads_mutex);

	DWORD id = GetCurrentThreadId();
	for (std::vector<Win32_Thread*>::iterator iter = g_win32_threads.begin(); iter != g_win32_threads.end(); iter++)
	{
		Win32_Thread* thread = *iter;
		if (thread->m_thread_id == id)
		{
			return thread;
		}
	}

	HANDLE handle;
	DuplicateHandle(GetCurrentProcess(),
					GetCurrentThread(), 
					GetCurrentProcess(),
					&handle,       
					0,                  
					TRUE,               
					DUPLICATE_SAME_ACCESS);

	Win32_Thread* thread = new Win32_Thread(handle, id);
	g_win32_threads.push_back(thread);

	return thread;
	*/
}

Win32_Thread::Win32_Thread(const char* name, EntryPoint entry_point, void* ptr) 
	: Thread(name) 
	, m_finished(false)
	, m_thread_id(0)
{
	m_entry_point = entry_point;
	m_entry_ptr = ptr;
	m_thread = CreateThread(NULL, NULL, Start_Routine, this, CREATE_SUSPENDED, &m_thread_id);

	DBG_ASSERT(m_thread != NULL);
}

Win32_Thread::Win32_Thread(HANDLE handle, DWORD id)
	: Thread(NULL)
	, m_finished(false)
	, m_thread_id(id)
{
	m_thread = handle;
	DBG_ASSERT(m_thread != NULL);
}

Win32_Thread::~Win32_Thread()
{
	// Wait for thread to terminate.
	while (!m_finished)
		Sleep(1.01f);

	// Close handle.
	int ret = CloseHandle(m_thread);
	DBG_ASSERT(ret != 0);
	m_thread = NULL;
}

DWORD WINAPI Win32_Thread::Start_Routine(LPVOID lpThreadParameter)
{
	Win32_Thread* thread = reinterpret_cast<Win32_Thread*>(lpThreadParameter);
	tls_self = thread;

	PROFILE_THREAD(thread->m_name.c_str());

	// Set thread name.
	if (thread->m_thread_id != 0 && thread->m_name.size() > 0)
	{
		Set_Visual_Studio_Thread_Name(thread->m_thread_id, thread->m_name.c_str());
	}

	// Call entry point.
	thread->m_entry_point(thread, thread->m_entry_ptr);
	thread->m_finished = true;

	return 0;
}

bool Win32_Thread::Is_Running()
{
	return !m_finished;
}

void Win32_Thread::Start()
{
	ResumeThread(m_thread);
}

void Win32_Thread::Join()
{
	WaitForSingleObject(m_thread, INFINITE);
}

void Win32_Thread::Set_Priority(ThreadPriority::Type priority)
{
	switch (priority)
	{
	case ThreadPriority::Highest:			SetThreadPriority(m_thread, THREAD_PRIORITY_HIGHEST);			return;	
	case ThreadPriority::High:				SetThreadPriority(m_thread, THREAD_PRIORITY_ABOVE_NORMAL);		return;	
	case ThreadPriority::Normal:			SetThreadPriority(m_thread, THREAD_PRIORITY_NORMAL);			return;	
	case ThreadPriority::Low:				SetThreadPriority(m_thread, THREAD_PRIORITY_BELOW_NORMAL);		return;	
	case ThreadPriority::Lowest:			SetThreadPriority(m_thread, THREAD_PRIORITY_LOWEST);			return;	
	default:
		DBG_ASSERT(false);
	}
}

void Win32_Thread::Set_Affinity(int mask)
{
	SetThreadAffinityMask(m_thread, mask);
}

void Win32_Thread::Sleep(float ms)
{
	::Sleep((DWORD)(ms));
}

int Win32_Thread::Get_Core_Count()
{
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);

	return sysinfo.dwNumberOfProcessors;
}

void Win32_Thread::Yield()
{
	::Sleep(0);
}

#endif