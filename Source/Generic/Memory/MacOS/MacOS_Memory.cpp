// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Memory/MacOS/MacOS_Memory.h"
#include "Generic/Memory/Memory.h"

#include <stdlib.h>
#include <malloc/malloc.h>
#include <time.h>
#include <CoreServices/CoreServices.h>
#include <mach/clock.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <time.h>

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
#endif

	return true;
}

double platform_get_ticks()
{
	uint64_t	time = mach_absolute_time();
	Nanoseconds nano = AbsoluteToNanoseconds(*(AbsoluteTime*)&time);
	uint64_t	ms	= (*(uint64_t*)&nano) / 1000000;

	return ms;
}

#endif