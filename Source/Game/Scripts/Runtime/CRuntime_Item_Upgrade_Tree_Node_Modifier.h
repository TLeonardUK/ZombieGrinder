// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_UPGRADE_TREE_NODE_MODIFIER_
#define _GAME_RUNTIME_ITEM_UPGRADE_TREE_NODE_MODIFIER_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item_Upgrade_Tree_Node_Modifier
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);

	static int Get_Modifier(CVirtualMachine* vm, CVMValue self);
	static void Set_Modifier(CVirtualMachine* vm, CVMValue self, int value);
	static float Get_Scale(CVirtualMachine* vm, CVMValue self);
	static void Set_Scale(CVirtualMachine* vm, CVMValue self, float value);

	static void Bind(CVirtualMachine* machine);

};

#endif
