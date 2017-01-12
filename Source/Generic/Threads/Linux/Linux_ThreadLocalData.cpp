// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_LINUX

#include "Generic/Threads/Atomic.h"
#include "Generic/Threads/Linux/Linux_ThreadLocalData.h"
#include <linux/unistd.h> 
#include <asm/ldt.h>

#include <pthread.h>

#define MAX_TLS_VALUES 32

pthread_key_t g_tls_keys[MAX_TLS_VALUES];

struct TlsAllocator
{
	int m_free_tls_indexes[MAX_TLS_VALUES];
	int m_free_tls_index_count;

	int Alloc()
	{
		DBG_ASSERT(m_free_tls_index_count > 0);
		int index = Atomic::Decrement32(&m_free_tls_index_count);
		return m_free_tls_indexes[index];
	}

	void Free(int i)
	{
		int index = Atomic::Increment32(&m_free_tls_index_count) - 1;
		m_free_tls_indexes[index] = i;
	}

	TlsAllocator()
	{
		for (int i = 0; i < MAX_TLS_VALUES; i++)
		{
			m_free_tls_indexes[i] = i;
			pthread_key_create(&g_tls_keys[i], NULL);
		}
		m_free_tls_index_count = MAX_TLS_VALUES;
	}

} g_tls_allocator;

Linux_ThreadLocalData::Linux_ThreadLocalData()
	: m_init(false)
{
}

Linux_ThreadLocalData::~Linux_ThreadLocalData()
{
	if (m_init)
	{
		g_tls_allocator.Free(m_tls_index);
		m_init = false;
	}
}

void Linux_ThreadLocalData::Init()
{
	m_tls_index = g_tls_allocator.Alloc();
	m_init = true;
}

void* Linux_ThreadLocalData::Get()
{
	if (!m_init)
	{
		Init();
	}
	return pthread_getspecific(g_tls_keys[m_tls_index]);
}

void Linux_ThreadLocalData::Set(void* value)
{
	if (!m_init)
	{
		Init();
	}
	pthread_setspecific(g_tls_keys[m_tls_index], value);
}


#endif