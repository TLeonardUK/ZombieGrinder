// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#ifndef _GENERIC_MACOS_THREADLOCALDATA_
#define _GENERIC_MACOS_THREADLOCALDATA_

#include "Generic/Threads/ThreadLocalData.h"

class MacOS_ThreadLocalData : public Platform_ThreadLocalData
{
	MEMORY_ALLOCATOR(MacOS_ThreadLocalData, "Platform");

private:
	friend class Platform_ThreadLocalData;

	int m_tls_index;
	int m_init;

	MacOS_ThreadLocalData();
	void Init();

public:
	~MacOS_ThreadLocalData();
	
	void* Get();
	void  Set(void*);

};

#endif

#endif