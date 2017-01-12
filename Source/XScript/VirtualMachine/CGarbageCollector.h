/* *****************************************************************

		CGarbageCollector.h

		Copyright (C) 2012 Tim Leonard - All Rights Reserved

   ***************************************************************** */
#pragma once
#ifndef _CGARBAGECOLLECTOR_H_
#define _CGARBAGECOLLECTOR_H_

#include "Generic/Types/String.h"
#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CVMValue.h"
#include <vector>

#include "Generic/Types/MemoryHeap.h"

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

#include <algorithm>
#include <string.h>

//#define GC_TRACK_ALLOCS
//#define GC_DUMP_STATS

#define GC_SMALL_ALLOC_THRESHOLD	  128
#define GC_SMALL_ALLOC_POOL_DIVISOR   3

// Any allocations <= 128 will be cached.
#define CACHE_LEVEL_INTERVAL 64			//
#define CACHE_LEVELS		 7			// How many levels of caches, max cached entry will be (CACHE_LEVEL_INTERVAL * CACHE_LEVELS)
#define CACHE_ENTRIES		 8192		// Max entries per cache level.

// If set the GC will use malloc rather than internally managed heaps. This should be fine
// as we shouldn't be allocating at any time except for during heavy loading, the cache
// should fill in for all frame allocations.
#define GC_USE_MALLOC

// =================================================================
//	A root object. Holds an object handle. Used by the GC to scan
//  for garbage and mark it.
//
//	Along with roots, the GC will also scan callstacks/registers.
// =================================================================
class CVMGCRoot
{
private:

#ifdef GC_USE_ARRAY_GLOBAL_LISTS
	static CVMGCRoot** m_global_list;
	static int m_global_list_size;
	static int m_global_list_capacity;
#else
	static CVMGCRoot* m_global_list;
	static int m_global_list_size;
#endif

	CVMObjectHandle m_handle;

	bool m_in_list;
	bool m_handle_marked_uncollectable;

	int m_index;

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	CVMGCRoot* m_next;
	CVMGCRoot* m_prev;
#endif

private:
	friend class CGarbageCollector;

#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	void Remove_Link()
	{
		if (m_prev != NULL)
			m_prev->m_next = m_next;
		if (m_next != NULL)
			m_next->m_prev = m_prev;

		if (this == m_global_list)
			m_global_list = m_next;

		m_global_list_size--;

//		DBG_ASSERT(!In_Chain(this));
//		Validate_Chain();
	}

	void Add_Link()
	{
		if (m_global_list != NULL)
			m_global_list->m_prev = this;

		this->m_next = m_global_list;
		this->m_prev = NULL;

		m_global_list = this;

		m_global_list_size++;

//		DBG_ASSERT(In_Chain(this));
//		Validate_Chain();
	}
#else

	void Remove_Link()
	{
		m_global_list[m_index] = m_global_list[m_global_list_size - 1];
		m_global_list[m_index]->m_index = m_index;
		m_global_list_size--;
		m_index = 0xFEDEFEDE;

		m_in_list = false;
	}

	void Add_Link()
	{
		if (m_global_list_size >= m_global_list_capacity)
		{
			int new_capacity = m_global_list_capacity * 2;

			if (new_capacity == 0)
			{
				new_capacity = 1024;
			}

			CVMGCRoot** new_list = new CVMGCRoot*[new_capacity];
			if (m_global_list != NULL)
			{
				memcpy(new_list, m_global_list, m_global_list_size * sizeof(CVMGCRoot*));
			}
			m_global_list = new_list;
			m_global_list_capacity = new_capacity;
		}

		m_index = m_global_list_size;
		m_global_list_size++;

		m_in_list = true;

		m_global_list[m_index] = this;
	}

#endif

public:
#ifndef GC_USE_ARRAY_GLOBAL_LISTS
	static void Validate_Chain();
	static bool In_Chain(CVMGCRoot* root);
#endif

	~CVMGCRoot()
	{
		Set_Handle(NULL);
	}

	CVMGCRoot()
#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		: m_next(NULL)
		, m_prev(NULL)
#else
		: m_in_list(false)
		, m_handle_marked_uncollectable(false)
#endif
	{
		Set_Handle(NULL);
	}

	// Arrrgh, we really shouldn't be doing copies on roots.
	CVMGCRoot(const CVMGCRoot& other)
#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		: m_next(NULL)
		, m_prev(NULL)
#else
		: m_in_list(false)
		, m_handle_marked_uncollectable(false)
#endif
	{
		Set_Handle(NULL);
	}

	CVMGCRoot(const CVMObjectHandle& handle)
#ifndef GC_USE_ARRAY_GLOBAL_LISTS
		: m_next(NULL)
		, m_prev(NULL)
#else
		: m_in_list(false)
		, m_handle_marked_uncollectable(false)
#endif
	{
		Set_Handle(handle);
	}

	//todo when assigning, if object cannot reference anything else, remove root from chain and mark object as uncollectable to
	//save time durin root enumeration. as most roots are assigned once and never changed

	CVMGCRoot& operator =(const CVMObjectHandle& other)
	{
		Set_Handle(other);
		return *this;
	}

	void Set_Handle(const CVMObjectHandle& other);

	INLINE CVMObjectHandle Get()
	{
		return m_handle;
	}

	INLINE CVMObjectHandle& GetRef()
	{
		return m_handle;
	}

	operator CVMObjectHandle() const
	{
		return m_handle;
	}

	operator CVMValue() const
	{
		return CVMValue(m_handle);
	}

};

// =================================================================
//	Manages all the memory allocated by a virutal machine.
// =================================================================
class CGarbageCollector
{
private:	
#ifndef GC_USE_MALLOC
	MemoryHeap m_heap_small;
	MemoryHeap m_heap_large;
#else
	int m_total_allocated;
	int m_lifetime_bytes_allocated;
#endif

	struct Cache
	{
		void*	   entries[CACHE_ENTRIES];
		int		   entry_count;
		int		   entry_size;
	};

	struct AllocHeader
	{
		int element_count;
	};

	Cache m_cache[CACHE_LEVELS];
	int m_max_cache_size;

	CVirtualMachine* m_vm;

	int m_traversed_objects;

	int m_skipped_traversals;
	int m_traverse_count;

public:
	CGarbageCollector(int size);
	~CGarbageCollector();

	void Init(CVirtualMachine* vm);

private:
	void* Alloc_Cached(int size);
	void Free_Cached(void* ptr);
	
	void Reintegrate_Cache();
	void Dump_Object_State();

public:
	void Traverse(CVMObject* object);
	void Traverse_Roots_In_Range(int start_index, int end_index);

public:

	void Collect();

	void* Alloc(int size);
	void Free(void* ptr);

#ifndef GC_USE_MALLOC
	MemoryHeap& Get_Heap(void* ptr);
	MemoryHeap& Get_Heap(int size);
#else
	void* HeaplessAlloc(int size);
	void HeaplessFree(void* ptr);
	int HeaplessSize(void* ptr);
#endif

	int Get_Total();
	int Get_Total_Used();
	int Get_Lifetime_Bytes_Allocated();

	// Unmanaged data is not collected. It has to be freed by the caller.
	template <typename T>
	INLINE T* Alloc_Unmanaged(int element_count = 1)
	{
		int size = sizeof(AllocHeader) + sizeof(T) * element_count;
		char* buffer = (char*)Alloc(size);
		DBG_ASSERT_STR(buffer != NULL, "Script GC heap ran out of memory (Used:%.1f mb/%.1f mb Alloc:%iB Elements:%i).", Get_Total_Used() / 1024.0f / 1024.0f, Get_Total() / 1024.0f / 1024.0f, size, element_count);

		AllocHeader* header = (AllocHeader*)buffer;
		header->element_count = element_count;

		T* base = (T*)(buffer + sizeof(AllocHeader));
		for (int i = 0; i < element_count; i++)
		{
			new (base + i) T();
		}

		return base;
	}

	template <typename T>
	INLINE void Free_Unmanaged(T* ptr)
	{		
		AllocHeader* header = (AllocHeader*)(((char*)ptr) - sizeof(AllocHeader));
		for (int i = 0; i < header->element_count; i++)
		{
			ptr[i].~T();
		}
		Free(header);
	}

};

#endif