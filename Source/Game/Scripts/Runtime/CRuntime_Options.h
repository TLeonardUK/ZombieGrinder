// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_OPTIONS_
#define _GAME_RUNTIME_OPTIONS_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Options
{
public:
	static int Get_Bool(CVirtualMachine* vm, CVMString name);

	static void Bind(CVirtualMachine* machine);
};

#endif
