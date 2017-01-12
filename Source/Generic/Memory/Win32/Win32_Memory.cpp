// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Memory/Win32/Win32_Memory.h"
#include "Generic/Memory/Memory.h"

#include <Windows.h>

extern int max_cpu_memory;

bool platform_init_memory()
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	static bool called = false;
	DBG_ASSERT_STR(!called, "platform_init_memory should only be called once!");
	called = true;

	void* mem = GlobalAlloc(GMEM_FIXED, max_cpu_memory);
	DBG_ASSERT(mem != NULL);
	platform_setup_heap(mem, max_cpu_memory);
#endif

	return true;
}

double platform_get_ticks()
{
	static ULONGLONG s_run_time_counter = GetTickCount();
	return (double)(GetTickCount() - s_run_time_counter);
}

#endif