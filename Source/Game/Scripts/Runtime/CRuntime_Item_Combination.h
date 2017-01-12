// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_COMBINATION_
#define _GAME_RUNTIME_ITEM_COMBINATION_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item_Combination
{
public:
	static CVMString Get_Other_Group(CVirtualMachine* vm, CVMValue self);
	static void Set_Other_Group(CVirtualMachine* vm, CVMValue self, CVMString value);
	static int Get_Combine_Method(CVirtualMachine* vm, CVMValue self);
	static void Set_Combine_Method(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Result_Item(CVirtualMachine* vm, CVMValue self);
	static void Set_Result_Item(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self, CVMString other, int method, CVMString result);

	static void Bind(CVirtualMachine* machine);
};

#endif
