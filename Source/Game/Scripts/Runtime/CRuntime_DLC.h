// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_DLC_
#define _GAME_RUNTIME_DLC_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_DLC
{
public:
	static void Set_ID(CVirtualMachine* vm, CVMValue self, int string);
	static int Get_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString string);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static int Get_Purchased(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif