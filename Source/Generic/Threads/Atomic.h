// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_ATOMIC_
#define _GENERIC_ATOMIC_

namespace Atomic
{
	// Increments int and returns incremented value.
	int Increment32(int* value);
	// Decrements int and returns decremented value.
	int Decrement32(int* value);
	// Adds int and returns summed value.
	int Add32(int* value, int inc);
	// Checks value is equal to initial_value if it is sets it to new_value. Return value is original value.
	int CompareExchange32(int* value, int initial_value, int new_value);
};

#endif

