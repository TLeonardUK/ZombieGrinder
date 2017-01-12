// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_MEMORY_
#define _GENERIC_MEMORY_

// Be careful what you put in this file! It's included by the Defines.h
// file which is included in every file in the project! Lotsofoverhead!

// Include memory allocator/management functions.
#include "Generic/Memory/MemoryManager.h"

#include <cstddef>         

class PooledMemoryHeap;

// Required implementations.
PooledMemoryHeap* platform_get_malloc_heap();
bool	platform_init_memory	();
void*	platform_malloc			(std::size_t size);
void*	platform_realloc		(void* ptr, std::size_t size);
void	platform_free			(void* ptr);
size_t	platform_memory_size	(void* ptr);
int		platform_get_used_memory();
int		platform_get_run_time	();
double	platform_get_ticks		();
void	platform_setup_heap		(void* ptr, int size);
void	platform_enable_allocation_tracking(bool enabled);

#endif
