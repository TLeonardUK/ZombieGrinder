// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#include "Generic/Threads/Win32/Win32_ThreadLocalData.h"

#include <windows.h>

Win32_ThreadLocalData::Win32_ThreadLocalData()
	: m_init(false)
{
}

Win32_ThreadLocalData::~Win32_ThreadLocalData()
{
	if (m_init)
	{
		TlsFree(m_tls_index);
		m_init = false;
	}
}

void Win32_ThreadLocalData::Init()
{
	m_tls_index = TlsAlloc();
	m_init = true;
	DBG_ASSERT(m_tls_index != TLS_OUT_OF_INDEXES);
}

void* Win32_ThreadLocalData::Get()
{
	if (!m_init)
	{
		Init();
	}
	return TlsGetValue(m_tls_index);
}

void Win32_ThreadLocalData::Set(void* value)
{
	if (!m_init)
	{
		Init();
	}
	TlsSetValue(m_tls_index, value);
}

#endif