// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_UPGRADE_TREE_NODE_
#define _GAME_RUNTIME_ITEM_UPGRADE_TREE_NODE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item_Upgrade_Tree_Node
{
public:
	static CVMObjectHandle Get_Position(CVirtualMachine* vm, CVMValue self);
	static void Set_Position(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_ID(CVirtualMachine* vm, CVMValue self);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMObjectHandle Get_Gem_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Gem_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_Connect_Up(CVirtualMachine* vm, CVMValue self);
	static void Set_Connect_Up(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Connect_Down(CVirtualMachine* vm, CVMValue self);
	static void Set_Connect_Down(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Connect_Left(CVirtualMachine* vm, CVMValue self);
	static void Set_Connect_Left(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Connect_Right(CVirtualMachine* vm, CVMValue self);
	static void Set_Connect_Right(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Cost(CVirtualMachine* vm, CVMValue self);
	static void Set_Cost(CVirtualMachine* vm, CVMValue self, int value);
	static CVMObjectHandle Get_Modifiers(CVirtualMachine* vm, CVMValue self);
	static void Set_Modifiers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);

	static void Bind(CVirtualMachine* machine);

};

#endif
