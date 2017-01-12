// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Memory/Linux/Linux_Memory.h"
#include "Generic/Memory/Memory.h"

#include <stdlib.h>
#include <malloc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <cmath>

int g_malloc_usage = 0;

extern int max_cpu_memory;

bool platform_init_memory()
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	static bool called = false;
	DBG_ASSERT_STR(!called, "platform_init_memory should only be called once!");
	called = true;

	void* mem = malloc(max_cpu_memory);
	DBG_ASSERT(mem != NULL);
	platform_setup_heap(mem, max_cpu_memory);

	//DBG_LOG("Initialiuzed platform memory block, base=0x%08x.", mem);
#endif

	return true;
}

double platform_get_ticks()
{
	timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);

	time_t seconds = time.tv_sec;
	u64 ms = ((u64)time.tv_nsec) / 1000000;

	double result = (seconds * 1000.0) + ms;
	
	static double initial_value = 0.0;
	static bool set_initial_value = false;

	if (!set_initial_value)
	{
		initial_value = result;
		set_initial_value = true;
	}

	return (result - initial_value);
}

#endif