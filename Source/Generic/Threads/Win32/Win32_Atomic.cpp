// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Atomic.h"

#include <windows.h>

namespace Atomic
{
	int Increment32(int* value)
	{
		return (int)InterlockedIncrement((LONG*)value);
	}
	int Decrement32(int* value)
	{
		return (int)InterlockedDecrement((LONG*)value);
	}
	int Add32(int* value, int inc)
	{
		return (int)InterlockedExchangeAdd((LONG*)value, (LONG)inc);
	}
	int CompareExchange32(int* value, int initial_value, int new_value)
	{
		return (int)InterlockedCompareExchange((LONG*)value, (LONG)new_value, (LONG)initial_value);
	}
};

#endif