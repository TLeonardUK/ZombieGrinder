/* *****************************************************************

		MemoryHeap.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _MEMORYHEAP_H_
#define _MEMORYHEAP_H_

#include "Generic/Threads/Mutex.h"
#include "Engine/Platform/Platform.h"

//#define USE_MEMORY_HEAP_GUARD_BYTES
//#define USE_HEAP_VALIDATION_DEFAULT_ON
//#define MEMORY_HEAP_GUARD_BYTE_COUNT 8
//#define MEMORY_HEAP_GUARD_BYTE_MAGIC 0x1F2E3D4C
//#define MEMORY_HEAP_LOG_CALLSTACKS
//#define MEMORY_HEAP_LOG_ENABLED_DEFAULT

#define MEMORY_HEAP_ALIGNMENT 16

// =================================================================
//	Block of data inside the memory heap.
// =================================================================
struct MemoryHeapBlock
{
#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	int guard_start[MEMORY_HEAP_GUARD_BYTE_COUNT];
#endif

	int		size;
	bool	used;
	char*	data;
	int		alloc_size;
	u32		alloc_index;
	bool	grouped;

	MemoryHeapBlock* next;
	MemoryHeapBlock* prev;

	MemoryHeapBlock* next_free;
	MemoryHeapBlock* prev_free;

#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	enum
	{
		max_callstack_frames = 16,
	};

	StackFrame callstack[max_callstack_frames];
	int callstack_size;

#endif

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	int guard_end[MEMORY_HEAP_GUARD_BYTE_COUNT];
#endif
};

// =================================================================
//	Simple memory heap allocator.
// =================================================================
class MemoryHeap
{
	MEMORY_ALLOCATOR(MemoryHeap, "Generic");

private:
	int				 m_total_allocations;
	int				 m_total_allocated_bytes;
	int				 m_lifetime_allocs;
	int				 m_start_size;
	char*			 m_memory;
	MemoryHeapBlock* m_first_block;

	MemoryHeapBlock  m_free_chain;

	//Mutex*			 m_mutex;

	bool			 m_owns_memory;

#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	bool			 m_logging_enabled;
#endif

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	bool			m_validate_heap;
#endif

	enum
	{
		minimum_block_size = 32,
		max_groups = 16
	};

	u32				m_groups[max_groups];
	int				m_group_index;
	u32				m_allocation_index;
		
protected:
	void Setup();
	void Set_Memory(int* ptr, int count, int expected_value);
	bool Validate_Memory(int* ptr, int count, int expected_value);
	void Validate_Heap();
	void Log_Block_Callstack(MemoryHeapBlock* block);
	void* Alloc_From_Block(MemoryHeapBlock* block, int size);
	void Log_Large_Allocations();
	void Free_Block(MemoryHeapBlock* block);

public:
	MemoryHeap(int size);
	MemoryHeap();
	~MemoryHeap();
	
#ifdef MEMORY_HEAP_LOG_CALLSTACKS
	void Enable_Callstack_Logging();
	void Disable_Callstack_Logging();
	void Dump_Callstack_Logging();
#endif

#ifdef USE_MEMORY_HEAP_GUARD_BYTES
	void Enable_Heap_Validation(bool enabled);
#endif

	void Push_Group();
	void Pop_Group();

	void Setup_With_Memory(char* mem, int size);
//	void Become_Reentrant();

	void Log_Stats();

	void* Alloc(int bytes);
	void* Realloc(void* ptr, int bytes);
	void Free(void* ptr);

	INLINE bool Contains(void* ptr)
	{
		return (ptr >= m_memory && ptr < m_memory + m_start_size);
	}

	int Get_Block_Size(void* block);
	int Get_Total_Used();
	int Get_Total();
	int Get_Lifetime_Bytes_Allocated();

};

#endif