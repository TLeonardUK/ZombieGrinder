// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_MACOS

#include "Generic/Threads/Atomic.h"
#include <libkern/OSAtomic.h>

namespace Atomic
{
	int Increment32(int* value)
	{
		return (int32_t)OSAtomicIncrement32((int32_t*)value);
	}
	int Decrement32(int* value)
	{
		return (int32_t)OSAtomicDecrement32((int32_t*)value);
	}
	int Add32(int* value, int inc)
	{
		return (int32_t)OSAtomicAdd32((int32_t)inc, (int32_t*)value);
	}
	int CompareExchange32(int* value, int initial_value, int new_value)
	{
		return (int32_t)OSAtomicCompareAndSwapInt((int32_t)initial_value, (int32_t)new_value, (int32_t*)value) ? initial_value : 0xFFFFFFFF;
	}
};

#endif