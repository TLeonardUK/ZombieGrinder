// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_CACHED_MEMORY_POOL_
#define _GENERIC_CACHED_MEMORY_POOL_

#include <typeinfo>

//  Cached memory pools allow you to allocate and release object memory 
//	but will cache released memory to prevent reallocation. Be aware that memory
//	is never freed until the pool is freed.
//
//	FixedMemoryPool<MyObjectClass> pool();
//
//	MyObjectClass obj1 = new(pool.Allocate()) MyObjectClass(derp, derp, derp);
//
//	pool.Release(obj1);
//
//
//	Memory blocks contain the following structure;
//
//		block_chain_next
//		free_chain_next
//		<data>

template <typename T>
struct CachedMemoryPool
{
	MEMORY_ALLOCATOR(CachedMemoryPool<T>, "Data Types");

private:
	int	  m_instance_size;
	char* m_block_head;
	char* m_free_head;
	int   m_pool_size;

public:

	CachedMemoryPool(int instance_size = 0)
	{
		if (instance_size == 0)
		{
			instance_size = sizeof(T);
		}

		m_instance_size = instance_size + sizeof(char*) + sizeof(char*);

		m_block_head = NULL;
		m_free_head	 = NULL;
		m_pool_size = 0;
	}

	~CachedMemoryPool()
	{
		Release_All();
	}

	void Release_All()
	{
		char* obj = m_block_head;
		while (obj != NULL)
		{
			char** block_chain_next_ptr = reinterpret_cast<char**>(obj);
			char** free_chain_next_ptr  = reinterpret_cast<char**>(obj + sizeof(char*));

			char* next = *block_chain_next_ptr;

			SAFE_DELETE_ARRAY(obj);

			obj = next;
		}

		m_block_head = NULL;
		m_free_head	 = NULL;
		m_pool_size = 0;
	}

	void* Allocate()
	{
		// Reallocate cached object?
		if (m_free_head != NULL)
		{
			char*  block				= m_free_head;
			char** block_chain_next_ptr = reinterpret_cast<char**>(block);
			char** free_chain_next_ptr  = reinterpret_cast<char**>(block + sizeof(char*));

			// Remove from free list.
			m_free_head = *free_chain_next_ptr;

			return block + sizeof(char*) + sizeof(char*);
		}

		// New object.
		else
		{
			char*  block				= new char[m_instance_size];
			char** block_chain_next_ptr = reinterpret_cast<char**>(block);
			char** free_chain_next_ptr  = reinterpret_cast<char**>(block + sizeof(char*));

			// Add to block chain.
			*block_chain_next_ptr = m_block_head;
			m_block_head = block;
			m_pool_size++;

			return block + sizeof(char*) + sizeof(char*);
		}

		return NULL;
	}

	void Release(void* obj)
	{	
		// Invoke destructor.	
		((T*)obj)->~T();

		char*  block				= reinterpret_cast<char*>(obj) - sizeof(char*) - sizeof(char*);
		char** block_chain_next_ptr = reinterpret_cast<char**>(block);
		char** free_chain_next_ptr  = reinterpret_cast<char**>(block + sizeof(char*));

		// Add to free list.
		*free_chain_next_ptr = m_free_head;
		m_free_head = block;
	}

};

#endif