// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Atomic.h"

namespace Atomic
{
	int Increment32(int* value)
	{
		return __sync_add_and_fetch(value, 1);
	}
	int Decrement32(int* value)
	{
		return __sync_add_and_fetch(value, -1);
	}
	int Add32(int* value, int inc)
	{
		return __sync_add_and_fetch(value, inc);
	}
	int CompareExchange32(int* value, int initial_value, int new_value)
	{
		return __sync_val_compare_and_swap(value, initial_value, new_value);
	}
};

#endif