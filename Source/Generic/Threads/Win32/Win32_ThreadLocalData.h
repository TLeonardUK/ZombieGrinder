// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _GENERIC_WIN32_THREADLOCALDATA_
#define _GENERIC_WIN32_THREADLOCALDATA_

#include "Generic/Threads/ThreadLocalData.h"
#include <windows.h>

class Win32_ThreadLocalData : public Platform_ThreadLocalData
{
	MEMORY_ALLOCATOR(Win32_ThreadLocalData, "Platform");

private:
	friend class Platform_ThreadLocalData;

	DWORD m_tls_index;
	int m_init;

	Win32_ThreadLocalData();
	void Init();

public:

	~Win32_ThreadLocalData();
	
	void* Get();
	void Set(void*);

};

#endif

#endif