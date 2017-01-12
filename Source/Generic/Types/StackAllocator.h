// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_FIXED_MEMORY_POOL_
#define _GENERIC_FIXED_MEMORY_POOL_

#include <typeinfo>
#include <cstring>

#include "Generic/Threads/Mutex.h"
#include "Generic/Threads/MutexLock.h"

template <typename T, int MaxAllocs>
struct ThreadSafeStackAllocator
{
	MEMORY_ALLOCATOR(ThreadSafeStackAllocator<T COMMA MaxAllocs>, "Data Types");

private:
	char* m_data;
	char* m_head;

	Mutex* m_mutex;

public:

	ThreadSafeStackAllocator()
	{
		m_mutex	= Mutex::Create();
		DBG_ASSERT(m_mutex != NULL);

		m_data = new char[sizeof(T) * MaxAllocs];
		m_head = m_data;
		memset(m_data, 0, sizeof(T) * MaxAllocs);
	}

	~ThreadSafeStackAllocator()
	{
		SAFE_DELETE_ARRAY(m_data);
		SAFE_DELETE(m_mutex);
	}

	T* Alloc()
	{
		MutexLock lock(m_mutex);

		if (Get_Count() >= MaxAllocs)
		{
			return NULL;
		}

		T* ptr = new(m_head) T;
		m_head += sizeof(T);

		return ptr;
	}

	void Free()
	{		
		MutexLock lock(m_mutex);

		while (m_head > m_data)
		{
			m_head -= sizeof(T);
			reinterpret_cast<T*>(m_head)->~T();
		}
	}

	int Get_Allocated()
	{
		return (int)(m_head - m_data);
	}

	T* Get_Base()
	{
		return reinterpret_cast<T*>(m_data);
	}

	int Get_Count()
	{
		return Get_Allocated() / sizeof(T);
	}

};

template <typename T, int MaxAllocs>
struct StackAllocator
{
	MEMORY_ALLOCATOR(StackAllocator<T COMMA MaxAllocs>, "Data Types");

private:
	char* m_data;
	char* m_head;

public:

	StackAllocator()
	{
		m_data = new char[sizeof(T) * MaxAllocs];
		m_head = m_data;
		memset(m_data, 0, sizeof(T) * MaxAllocs);
	}

	~StackAllocator()
	{
		SAFE_DELETE_ARRAY(m_data);
	}

	T* Alloc()
	{
		if (Get_Count() >= MaxAllocs)
		{
			DBG_LOG("Ran out of stack space! %i allocated.", Get_Count());
			return NULL;
		}

		T* ptr = new(m_head) T;
		m_head += sizeof(T);

		return ptr;
	}

	void Free()
	{		
		while (m_head > m_data)
		{
			m_head -= sizeof(T);
			reinterpret_cast<T*>(m_head)->~T();
		}
	}

	int Get_Allocated()
	{
		return (int)(m_head - m_data);
	}

	T* Get_Base()
	{
		return reinterpret_cast<T*>(m_data);
	}

	int Get_Count()
	{
		return Get_Allocated() / sizeof(T);
	}

};

#endif