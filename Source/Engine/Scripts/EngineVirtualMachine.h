// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _ENGINE_VIRTUALMACHINE_
#define _ENGINE_VIRTUALMACHINE_

#include "XScript/VirtualMachine/CVirtualMachine.h"

class Mutex;

class EngineVirtualMachine
	: public CVirtualMachine
{
	MEMORY_ALLOCATOR(EngineVirtualMachine, "Engine");

protected:
	virtual void Register_Runtime();

	bool m_locked;

	Mutex* m_link_mutex;

	float m_register_progress;

public:
	EngineVirtualMachine();
	~EngineVirtualMachine();

	void Update_Statistics();

	void Lock();
	void Unlock();

	float Get_Register_Progress() { return m_register_progress; }

	void Execute_All_Contexts(float time);

//	void Link_Script(CVMBinary* binary);
	void Register_Scripts();

};

#endif

