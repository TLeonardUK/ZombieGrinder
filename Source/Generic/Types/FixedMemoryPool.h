// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_FIXED_MEMORY_POOL_
#define _GENERIC_FIXED_MEMORY_POOL_

#include <typeinfo>
#include <cstring>

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

// Fixed memory pools allow you to preallocate a fixed number of fixed sized
// blocks of memory. This is used to preallocate memory for objects early on
// and instantiate objects using placement new. eg.
//
//	FixedMemoryPool<MyObjectClass> pool(100);
//
//	MyObjectClass obj1 = new(pool.Allocate()) MyObjectClass(derp, derp, derp);
//
//	pool.Release(obj1);
//
template <typename T>
struct FixedMemoryPool
{
	MEMORY_ALLOCATOR(FixedMemoryPool<T>, "Data Types");

private:
	int	  m_stride;
	int	  m_size;
	int	  m_allocated;
	char* m_blocks;
	char* m_blocks_end;
	char* m_free_head;

	Mutex* m_mutex;

public:

	FixedMemoryPool(int size, int instance_size = 0)
	{
		if (instance_size == 0)
		{
			instance_size = sizeof(T);
		}

		// Create mutex.
		m_mutex					= Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);

		m_size					= size;
		m_allocated				= 0;
		m_stride				= instance_size + sizeof(char*);

		m_blocks = new char[size * m_stride];
		memset(m_blocks, 0, size * m_stride);

		m_blocks_end = m_blocks + (size * m_stride);

		// Setup free-chain.
		m_free_head	= m_blocks;
		for (int i = 0; i < size; i++)
		{
			char* block		 = m_blocks + (i * m_stride);
			char* next_block = (i + 1 >= size) ? NULL : m_blocks + ((i + 1) * m_stride);

			char** next_block_ptr = reinterpret_cast<char**>(block);
			*next_block_ptr = next_block;
		}

		DBG_LOG("FixedMemoryPool<%s> preallocated %.02fmb bytes.", typeid(T).name(), ((size * m_stride) / 1024.0f) / 1024.0f);
	}

	~FixedMemoryPool()
	{
		SAFE_DELETE_ARRAY(m_blocks);
		SAFE_DELETE(m_mutex);
	}

	int Get_Free_Block_Count()
	{
		MutexLock lock(m_mutex);
		return m_size - m_allocated;
	}

	void* Allocate()
	{
		MutexLock lock(m_mutex);

		if (m_allocated >= m_size)
		{
			return NULL;
		}

		char* block	= m_free_head;
		char** next_block_ptr = reinterpret_cast<char**>(block);
		m_free_head = *next_block_ptr;
		
		m_allocated++;

		return block + sizeof(char*);
	}

	void Release(void* obj)
	{		
		MutexLock lock(m_mutex);

		DBG_ASSERT(m_allocated > 0);

		// Invoke destructor.	
		((T*)obj)->~T();

		char*  block		  = (char*)obj - sizeof(char*);
		char** next_block_ptr = reinterpret_cast<char**>(block);

		*next_block_ptr = m_free_head;
		m_free_head = block;

		m_allocated--;
	}

};

#endif