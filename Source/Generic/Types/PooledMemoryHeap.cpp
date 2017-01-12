/* *****************************************************************

		PooledMemoryHeap.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#include "Generic/Types/PooledMemoryHeap.h"
#include "Generic/Threads/MutexLock.h"

#include "Generic/Math/Math.h"

#include <cstring>

PooledMemoryHeap::PooledMemoryHeap()
{
}

PooledMemoryHeap::~PooledMemoryHeap()
{
}

void PooledMemoryHeap::Init()
{
	m_pool_count = 0;
	m_mutex = NULL;
}

void PooledMemoryHeap::Add_Pool(char* memory, int memory_size, int threshold)
{
	MemoryHeapPool& pool = m_pools[m_pool_count++];
	pool.heap.Setup_With_Memory(memory, memory_size);
	pool.threshold = threshold;
}
	
void PooledMemoryHeap::Push_Group()
{
	for (int i = 0; i < m_pool_count; i++)
		m_pools[i].heap.Push_Group();
}

void PooledMemoryHeap::Pop_Group()
{
	for (int i = 0; i < m_pool_count; i++)
		m_pools[i].heap.Pop_Group();
}

void PooledMemoryHeap::Become_Reentrant()
{
	DBG_ASSERT(m_mutex == NULL);
	m_mutex = Mutex::Create();
}

void* PooledMemoryHeap::Alloc(int bytes)
{
	MutexLock lock(m_mutex);
	
	for (int i = 0; i < m_pool_count; i++)
	{
		MemoryHeapPool& pool = m_pools[i];
		if (pool.threshold == 0 || bytes <= pool.threshold)
		{
			void* ptr = pool.heap.Alloc(bytes + sizeof(int));
			if (ptr != NULL)
			{
				*reinterpret_cast<int*>(ptr) = i;
				return ((char*)ptr) + sizeof(int);
			}
		}
	}
	
	for (int i = 0; i < m_pool_count; i++)
		m_pools[i].heap.Log_Stats();

	DBG_ASSERT_STR(false, "Failed to allocate %i bytes.", bytes);
	return NULL;
}

void* PooledMemoryHeap::Realloc(void* ptr, int bytes)
{
	MutexLock lock(m_mutex);
	
	Free(ptr);
	return Alloc(bytes);

	/*
	char* base = ((char*)ptr - sizeof(int));
	int pool_index = *reinterpret_cast<int*>(base);
	
	int original_size = m_pools[pool_index].heap.Get_Block_Size(base);
	
	if (bytes <= original_size - sizeof(int))
	{
		return ptr;
	}

	void* new_ptr = Alloc(bytes);
	memcpy(new_ptr, ptr, Min(original_size - sizeof(int), bytes));

	m_pools[pool_index].heap.Free(base);
	
	return new_ptr;
	*/

	/*
	char* base = ((char*)ptr - 1);
	int pool_index = base[0];
	ptr = m_pools[pool_index].heap.Realloc(base, bytes + 1);

	if (ptr == NULL)
	{
		return Alloc(bytes);
	}
	else if (ptr != NULL)
	{
		base = (char*)ptr;
		base[0] = pool_index;
		return base + 1;
	}
	
	DBG_ASSERT_STR(false, "Failed to reallocate %i bytes.", bytes);
	return NULL;
	*/
}

void PooledMemoryHeap::Free(void* ptr)
{
	MutexLock lock(m_mutex);

	char* base = ((char*)ptr - sizeof(int));
	int pool_index = *reinterpret_cast<int*>(base);
	DBG_ASSERT(pool_index >= 0 && pool_index < m_pool_count);

	m_pools[pool_index].heap.Free(base);
}

int PooledMemoryHeap::Get_Block_Size(void* ptr)
{
	MutexLock lock(m_mutex);

	char* base = ((char*)ptr - sizeof(int));
	int pool_index = *reinterpret_cast<int*>(base);
	DBG_ASSERT(pool_index >= 0 && pool_index < m_pool_count);

	return m_pools[pool_index].heap.Get_Block_Size(base);
}

int PooledMemoryHeap::Get_Total_Used()
{
	MutexLock lock(m_mutex);

	int total = 0;	
	for (int i = 0; i < m_pool_count; i++)
	{
		total += m_pools[i].heap.Get_Total_Used();	
	}

	return total;
}

int PooledMemoryHeap::Get_Total()
{
	MutexLock lock(m_mutex);

	int total = 0;	
	for (int i = 0; i < m_pool_count; i++)
	{
		total += m_pools[i].heap.Get_Total();	
	}

	return total;
}

int PooledMemoryHeap::Get_Lifetime_Bytes_Allocated()
{
	MutexLock lock(m_mutex);

	int total = 0;	
	for (int i = 0; i < m_pool_count; i++)
	{
		total += m_pools[i].heap.Get_Lifetime_Bytes_Allocated();	
	}

	return total;
}