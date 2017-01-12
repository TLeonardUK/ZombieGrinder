// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_UPGRADE_TREE_
#define _GAME_RUNTIME_ITEM_UPGRADE_TREE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item_Upgrade_Tree
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);

	static CVMObjectHandle Get_Active_Compound_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle upgrade_ids);

	static CVMObjectHandle Get_Nodes(CVirtualMachine* vm, CVMValue self);
	static void Set_Nodes(CVirtualMachine* vm, CVMValue self, CVMObjectHandle nodes);

	static CVMObjectHandle Find_By_Type(CVirtualMachine* vm, CVMObjectHandle type);

	static void Bind(CVirtualMachine* machine);

};

#endif
