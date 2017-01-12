// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Generic/Memory/Memory.h"
#include "Generic/Types/PooledMemoryHeap.h"
#include <new>
#include <cstring>
#include <stdio.h>

#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
static char					g_malloc_heap_block[sizeof(PooledMemoryHeap)];
static PooledMemoryHeap*	g_malloc_heap = NULL;
#endif

static bool					g_allocation_tracking_enabled = true;

void platform_enable_allocation_tracking(bool enabled)
{
	g_allocation_tracking_enabled = enabled;
}

INLINE void platform_memory_setup_trap()
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	if ((char*)g_malloc_heap != (char*)g_malloc_heap_block)
	{
		//	DBG_LOG("Memory allocation setup trap tripped.");
		memset(g_malloc_heap_block, 0, sizeof(PooledMemoryHeap));
		g_malloc_heap = reinterpret_cast<PooledMemoryHeap*>(g_malloc_heap_block);
		g_malloc_heap->Init();
		platform_init_memory();
	}
#endif
}

void platform_setup_heap(void* ptr, int size)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	//#ifndef MASTER_BUILD
	memset(ptr, 0, size);
	//#endif

	// 64th ~5mb for average pool of 300mb
	int pool_1 = (size / 64) * 7;	// 35mb
	int pool_2 = (size / 64) * 2;	// 20mb
	int pool_3 = (size / 64) * 2;	// 10mb
	int pool_4 = (size / 64) * 53;	// 235mb

	g_malloc_heap->Add_Pool((char*)ptr,									pool_1, 64);
	g_malloc_heap->Add_Pool((char*)ptr + pool_1,						pool_2, 256);
	g_malloc_heap->Add_Pool((char*)ptr + pool_1 + pool_2,				pool_3, 64 * 1024);
	g_malloc_heap->Add_Pool((char*)ptr + pool_1 + pool_2 + pool_3,		pool_4);
#endif
}

PooledMemoryHeap* platform_get_malloc_heap()
{
	platform_memory_setup_trap();

#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	return g_malloc_heap;
#else
	return NULL;
#endif
}

void* platform_malloc(std::size_t size)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_memory_setup_trap();
	void* ptr = g_malloc_heap->Alloc(size);
	if (g_allocation_tracking_enabled)
	{
		printf("Malloc 0x%08x - %i\n", ptr, size);
	}
	DBG_ASSERT(ptr != NULL);
	return ptr;
#else
	return malloc(size);
#endif
}

void* platform_realloc(void* ptr, std::size_t size)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_memory_setup_trap();
	ptr = g_malloc_heap->Realloc(ptr, size);
	if (g_allocation_tracking_enabled)
	{
		printf("Realloc 0x%08x - %i -> %i\n", ptr, platform_memory_size(ptr), size);
	}
	DBG_ASSERT(ptr != NULL);
	return ptr;
#else
	return realloc(ptr, size);
#endif
}

void platform_free(void* ptr)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_memory_setup_trap();
	if (g_allocation_tracking_enabled)
	{
		printf("Free 0x%08x - %i\n", ptr, platform_memory_size(ptr));
	}
	g_malloc_heap->Free(ptr);
#else
	free(ptr);
#endif
}

size_t platform_memory_size(void* ptr)
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_memory_setup_trap();
	return g_malloc_heap->Get_Block_Size(ptr);
#else
	return 0;
#endif
}

int platform_get_used_memory()
{
#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS
	platform_memory_setup_trap();
	return g_malloc_heap->Get_Total_Used();
#else
	return 0;
#endif
}

int	platform_get_run_time()
{
	static int start_time = (int)platform_get_ticks();
	return (int)((int)platform_get_ticks() - start_time);
}


#ifdef OPT_OVERRIDE_DEFAULT_ALLOCATORS

void* operator new(std::size_t count)
{
	return MemoryManager::Get_System_Allocator()->Alloc(count);
}

void* operator new[](std::size_t count)
{
	return MemoryManager::Get_System_Allocator()->Alloc(count);
}

void* operator new(std::size_t count, const std::nothrow_t&)
{
	return MemoryManager::Get_System_Allocator()->Alloc(count);
}

void* operator new[](std::size_t count, const std::nothrow_t&)
{
	return MemoryManager::Get_System_Allocator()->Alloc(count);
}
	
void operator delete(void* ptr) NO_EXCEPT(true)
{
	if (ptr != NULL)	// C++ standard requires that delete handle null pointers.
		MemoryManager::Get_System_Allocator()->Free(ptr);
}

void operator delete[](void* ptr) NO_EXCEPT(true)
{
	if (ptr != NULL)	// C++ standard requires that delete handle null pointers.
		MemoryManager::Get_System_Allocator()->Free(ptr);
}

void operator delete(void* ptr, const std::nothrow_t&)
{
	if (ptr != NULL)	// C++ standard requires that delete handle null pointers.
		MemoryManager::Get_System_Allocator()->Free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&)
{	
	if (ptr != NULL)	// C++ standard requires that delete handle null pointers.
		MemoryManager::Get_System_Allocator()->Free(ptr);
}

#endif