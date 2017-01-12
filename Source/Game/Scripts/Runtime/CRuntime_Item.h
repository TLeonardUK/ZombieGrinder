// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_
#define _GAME_RUNTIME_ITEM_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item
{
public:
	static CVMObjectHandle Create(CVirtualMachine* vm, CVMValue self);
	static CVMObjectHandle Get_Archetype(CVirtualMachine* vm, CVMValue self);
	static void Set_Archetype(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_Archetype_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Archetype_ID(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Unique_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Unique_ID(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Recieve_Time(CVirtualMachine* vm, CVMValue self);
	static void Set_Recieve_Time(CVirtualMachine* vm, CVMValue self, int value);
	static CVMObjectHandle Get_Primary_Color(CVirtualMachine* vm, CVMValue self);
	static void Set_Primary_Color(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_Equip_Time(CVirtualMachine* vm, CVMValue self);
	static void Set_Equip_Time(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Equip_Slot(CVirtualMachine* vm, CVMValue self);
	static void Set_Equip_Slot(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Attached_To_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Attached_To_ID(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Is_Indestructable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Indestructable(CVirtualMachine* vm, CVMValue self, int value);

	static float Get_Equip_Duration(CVirtualMachine* vm, CVMValue self);
	static void Set_Equip_Duration(CVirtualMachine* vm, CVMValue self, float value);

	static int Get_Was_Inventory_Drop(CVirtualMachine* vm, CVMValue self);
	static void Set_Was_Inventory_Drop(CVirtualMachine* vm, CVMValue self, int value);

	static CVMObjectHandle Get_Upgrade_IDs(CVirtualMachine* vm, CVMValue self);
	static void Set_Upgrade_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMString Get_Inventory_Drop_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Inventory_Drop_ID(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Inventory_Original_Drop_ID(CVirtualMachine* vm, CVMValue self);
	static void Set_Inventory_Original_Drop_ID(CVirtualMachine* vm, CVMValue self, CVMString value);
	
	static void Bind(CVirtualMachine* machine);
};

#endif
