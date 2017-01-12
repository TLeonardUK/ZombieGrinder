/* *****************************************************************

		PooledMemoryHeap.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _POOLEDMEMORYHEAP_H_
#define _POOLEDMEMORYHEAP_H_

#include "Generic/Types/MemoryHeap.h"

class Mutex;

struct MemoryHeapPool
{
	MemoryHeap  heap;
	int			threshold;
};

// =================================================================
//	Pooled allocator. Seperates allocations into different pools
//  based on size to reduce fragmentation.
// =================================================================
class PooledMemoryHeap
{
private:
	enum
	{
		max_pools = 8,
	};

	MemoryHeapPool m_pools[max_pools];
	int m_pool_count;

	Mutex* m_mutex;

protected:

public:
	PooledMemoryHeap();
	~PooledMemoryHeap();

	void Init();

	void Add_Pool(char* memory, int memory_size, int threshold = 0);
	
	void Push_Group();
	void Pop_Group();

	void Become_Reentrant();

	void* Alloc(int bytes);
	void* Realloc(void* ptr, int bytes);
	void Free(void* ptr);

	int Get_Block_Size(void* block);
	int Get_Total_Used();
	int Get_Total();
	int Get_Lifetime_Bytes_Allocated();

};



#endif