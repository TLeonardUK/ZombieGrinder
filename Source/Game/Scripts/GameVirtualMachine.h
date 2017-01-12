// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_VIRTUALMACHINE_
#define _GAME_VIRTUALMACHINE_

#include "Engine/Scripts/EngineVirtualMachine.h"

class GameVirtualMachine
	: public EngineVirtualMachine
{
	MEMORY_ALLOCATOR(GameVirtualMachine, "Engine");

protected:
	virtual void Register_Runtime();

public:
	GameVirtualMachine();
	~GameVirtualMachine();

};

#endif

