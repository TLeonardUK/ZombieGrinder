// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_INPUT_
#define _GAME_RUNTIME_INPUT_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Input
{
public:
	static int Is_Down(CVirtualMachine* vm, CVMObjectHandle net_user, int key_binding);
	static int Was_Pressed(CVirtualMachine* vm, CVMObjectHandle net_user, int key_binding);

	static void Bind(CVirtualMachine* machine);
};

#endif
