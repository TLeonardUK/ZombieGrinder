// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_RUNTIME_PROFILE_
#define _GAME_RUNTIME_PROFILE_

#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMValue.h"

class CRuntime_Profile
{
public:
	static CVMString Get_Name(CVirtualMachine* vm, CVMValue self);
	static void Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value);
	static int Get_Level(CVirtualMachine* vm, CVMValue self);
	static void Set_Level(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Level_XP(CVirtualMachine* vm, CVMValue self);
	static void Set_Level_XP(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Next_Level_XP(CVirtualMachine* vm, CVMValue self);
	static int Get_Coins(CVirtualMachine* vm, CVMValue self);
	static void Set_Coins(CVirtualMachine* vm, CVMValue self, int value);
	static float Get_Coins_Interest(CVirtualMachine* vm, CVMValue self);
	static void Set_Coins_Interest(CVirtualMachine* vm, CVMValue self, float value);
	static int Get_Skill_Points(CVirtualMachine* vm, CVMValue self);
	static void Set_Skill_Points(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Inventory_Size(CVirtualMachine* vm, CVMValue self);
	static void Set_Inventory_Size(CVirtualMachine* vm, CVMValue self, int value);
	static int Get_Wallet_Size(CVirtualMachine* vm, CVMValue self);
	static void Set_Wallet_Size(CVirtualMachine* vm, CVMValue self, int value);
	static float Get_Wallet_Interest(CVirtualMachine* vm, CVMValue self);
	static void Set_Wallet_Interest(CVirtualMachine* vm, CVMValue self, float value);
	static float Get_Sell_Back_Factor(CVirtualMachine* vm, CVMValue self);
	static void Set_Sell_Back_Factor(CVirtualMachine* vm, CVMValue self, float value);
	
	static int Get_Is_Male(CVirtualMachine* vm, CVMValue self);
	static void Set_Is_Male(CVirtualMachine* vm, CVMValue self, int value);
		
	static CVMObjectHandle Get_Skills(CVirtualMachine* vm, CVMValue self);
	static void Set_Skills(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Stat_Multipliers(CVirtualMachine* vm, CVMValue self);
	static void Set_Stat_Multipliers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Get_Items(CVirtualMachine* vm, CVMValue self);
	static void Set_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Get_Skill_Slot(CVirtualMachine* vm, CVMValue self, int index);

	static int Has_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Unlock_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);	
	
	
	static int Get_Inventory_Space(CVirtualMachine* vm, CVMValue self);
	
	static int Has_Space_For_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static int Has_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Add_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static void Remove_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int bConsume);
	static CVMObjectHandle Get_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Preferred_Consume_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Item_Slot(CVirtualMachine* vm, CVMValue self, int slot);

	
	static void Equip_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static void Level_Up(CVirtualMachine* vm, CVMValue self);
	static int Give_XP(CVirtualMachine* vm, CVMValue self, int amount);
	static int Give_Fractional_XP(CVirtualMachine* vm, CVMValue self, float amount);
	static void Combine_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle item_a, CVMObjectHandle item_b);
	static CVMObjectHandle Get_Attached_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other_item);

	static void Add_Unpacked_Fractional_XP(CVirtualMachine* vm, CVMValue self, float amount);
	static void Add_Unpacked_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static void Add_Unpacked_Coins(CVirtualMachine* vm, CVMValue self, int coins);

	static CVMObjectHandle Get_Applied_DLC_IDs(CVirtualMachine* vm, CVMValue self);
	static void Set_Applied_DLC_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);

	static CVMObjectHandle Get_Unlocked_Item_IDs(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlocked_Item_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static CVMObjectHandle Get_Unlocked_Skill_IDs(CVirtualMachine* vm, CVMValue self);
	static void Set_Unlocked_Skill_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);


	static CVMObjectHandle Get_Profile_Item_Stats(CVirtualMachine* vm, CVMValue self);
	static void Set_Profile_Item_Stats(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static float Get_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat);
	static void Set_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat, float amount);
	static void Increment_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat, float amount);



	static CVMObjectHandle Get_Profile_Skill_Energies(CVirtualMachine* vm, CVMValue self);
	static void Set_Profile_Skill_Energies(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static float Get_Skill_Energy(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value);
	static void Set_Skill_Energy(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, float amount);
	static void Increment_Skill_Energies(CVirtualMachine* vm, CVMValue self, int stat, float amount);



	static void Combine_Stat_Multipliers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle output_array, CVMObjectHandle skill_multiplier_array, int bAllowHealthRegen, int bAllowAmmoRegen);

	static int Get_Unique_ID_Counter(CVirtualMachine* vm, CVMValue self);
	static void Set_Unique_ID_Counter(CVirtualMachine* vm, CVMValue self, int value);
	static void Bind(CVirtualMachine* machine);
};

#endif
