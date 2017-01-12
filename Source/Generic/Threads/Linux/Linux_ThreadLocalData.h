// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#ifndef _GENERIC_LINUX_THREADLOCALDATA_
#define _GENERIC_LINUX_THREADLOCALDATA_

#include "Generic/Threads/ThreadLocalData.h"

class Linux_ThreadLocalData : public Platform_ThreadLocalData
{
	MEMORY_ALLOCATOR(Linux_ThreadLocalData, "Platform");

private:
	friend class Platform_ThreadLocalData;

	Linux_ThreadLocalData();
	void Init();

	int m_init;
	int m_tls_index;

public:

	~Linux_ThreadLocalData();

	void* Get();
	void Set(void*);

};

#endif

#endif