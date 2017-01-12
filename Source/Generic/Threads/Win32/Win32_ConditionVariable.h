// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifdef PLATFORM_WIN32

#ifndef _GENERIC_WIN32_CONDITIONVARIABLE_
#define _GENERIC_WIN32_CONDITIONVARIABLE_

#include "Generic/Threads/ConditionVariable.h"

#include <Windows.h>

class Win32_ConditionVariable : public ConditionVariable
{
	MEMORY_ALLOCATOR(Win32_ConditionVariable, "Platform");

private:
	friend class ConditionVariable;
	Win32_ConditionVariable();
		
	//CONDITION_VARIABLE m_convar_handle;
		
	HANDLE m_handle;

public:
	~Win32_ConditionVariable();
	
	void Broadcast();
	void Signal();
	bool Wait(Mutex* mutex, unsigned int timeout);

};

#endif

#endif