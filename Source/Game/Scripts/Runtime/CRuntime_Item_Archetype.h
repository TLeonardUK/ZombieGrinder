// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_ITEM_ARCHETYPE_
#define _GAME_RUNTIME_ITEM_ARCHETYPE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Item_Archetype
{
public:
	static CVMString Get_Varient_Prefix(CVirtualMachine* vm, CVMValue self);
	static void Set_Varient_Prefix(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Description(CVirtualMachine* vm, CVMValue self);
	static void Set_Description(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Category(CVirtualMachine* vm, CVMValue self);
	static void Set_Category(CVirtualMachine* vm, CVMValue self, CVMString value);
	static int Get_Slot(CVirtualMachine* vm, CVMValue self);
	static void Set_Slot(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Cost(CVirtualMachine* vm, CVMValue self);
	static void Set_Cost(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Stackable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Stackable(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Buyable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Buyable(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Tintable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Tintable(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Is_Unpackable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Unpackable(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Is_Sellable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Sellable(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Use_Custom_Color(CVirtualMachine* vm, CVMValue self);
	static void Set_Use_Custom_Color(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Unlock_Criteria(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria_Threshold(CVirtualMachine* vm, CVMValue self, int value);
	static CVMObjectHandle Get_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlock_Criteria_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	
	static int Get_Is_Tradable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Tradable(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Dropable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Dropable(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Inventory_Dropable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Inventory_Dropable(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Max_Stack(CVirtualMachine* vm, CVMValue self);
	static void Set_Max_Stack(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Rarity(CVirtualMachine* vm, CVMValue self);
	static void Set_Rarity(CVirtualMachine* vm, CVMValue self, int value);
	static CVMObjectHandle Get_Default_Tint(CVirtualMachine* vm, CVMValue self);
	static void Set_Default_Tint(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMString Get_Icon_Animation(CVirtualMachine* vm, CVMValue self);
	static void Set_Icon_Animation(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Body_Animation(CVirtualMachine* vm, CVMValue self);
	static void Set_Body_Animation(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMObjectHandle Get_Icon_Offset(CVirtualMachine* vm, CVMValue self);
	static void Set_Icon_Offset(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Get_Is_Icon_Direction_Based(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Icon_Direction_Based(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Is_Icon_Overlay(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Icon_Overlay(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Combine_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Combine_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMString Get_Combine_Group(CVirtualMachine* vm, CVMValue self);
	static void Set_Combine_Group(CVirtualMachine* vm, CVMValue self, CVMString value);
	static CVMObjectHandle Get_Combinations(CVirtualMachine* vm, CVMValue self);
	static void Set_Combinations(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Find_By_Name(CVirtualMachine* vm, CVMString name);
	static CVMObjectHandle Find_By_Type(CVirtualMachine* vm, CVMObjectHandle name);
	static int Get_Is_PVP_Usable(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_PVP_Usable(CVirtualMachine* vm, CVMValue self, int value);

	static int Get_Is_Premium_Only(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Premium_Only(CVirtualMachine* vm, CVMValue self, int value);
	static CVMString Get_Premium_Price(CVirtualMachine* vm, CVMValue self);
	static void Set_Premium_Price(CVirtualMachine* vm, CVMValue self, CVMString value);

	static float Get_Max_Durability(CVirtualMachine* vm, CVMValue self);
	static void Set_Max_Durability(CVirtualMachine* vm, CVMValue self, float value);

	static float Get_Initial_Armour_Amount(CVirtualMachine* vm, CVMValue self);
	static void Set_Initial_Armour_Amount(CVirtualMachine* vm, CVMValue self, float value);

	static CVMObjectHandle Get_Weapon_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Weapon_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	
	static CVMObjectHandle Get_Ammo_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Ammo_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Get_Post_Process_FX_Type(CVirtualMachine* vm, CVMValue self);
	static void Set_Post_Process_FX_Type(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Get_Upgrade_Tree(CVirtualMachine* vm, CVMValue self);
	static void Set_Upgrade_Tree(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMString Get_Override_Inventory_ID_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Override_Inventory_ID_Name(CVirtualMachine* vm, CVMValue self, CVMString value);

	static void Bind(CVirtualMachine* machine);
};

#endif
