// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Scripts/Runtime/CRuntime_Profile.h"
#include "XScript/VirtualMachine/CVMBindingHelper.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"

CVMString CRuntime_Profile::Get_Name(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Name.c_str();
}

void CRuntime_Profile::Set_Name(CVirtualMachine* vm, CVMValue self, CVMString value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Name = value.C_Str();
}

int CRuntime_Profile::Get_Level(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Level;
}

void CRuntime_Profile::Set_Level(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Level = value;
}

int CRuntime_Profile::Get_Level_XP(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Level_XP;
}

void CRuntime_Profile::Set_Level_XP(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Level_XP = value;
}

int CRuntime_Profile::Get_Next_Level_XP(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return ProfileManager::Get()->Get_Level_XP(profile->Level + 1);
}

int CRuntime_Profile::Get_Coins(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Coins;
}

void CRuntime_Profile::Set_Coins(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Coins = value;
}

float CRuntime_Profile::Get_Coins_Interest(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Coins_Interest;
}

void CRuntime_Profile::Set_Coins_Interest(CVirtualMachine* vm, CVMValue self, float value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Coins_Interest = value;
}

int CRuntime_Profile::Get_Skill_Points(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Skill_Points;
}

void CRuntime_Profile::Set_Skill_Points(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Skill_Points = value;
}

int CRuntime_Profile::Get_Inventory_Size(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Inventory_Size;
}

void CRuntime_Profile::Set_Inventory_Size(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Inventory_Size = value;
}

int CRuntime_Profile::Get_Wallet_Size(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Wallet_Size;
}

void CRuntime_Profile::Set_Wallet_Size(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Wallet_Size = value;
}

int CRuntime_Profile::Get_Unique_ID_Counter(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Unique_ID_Counter;
}

void CRuntime_Profile::Set_Unique_ID_Counter(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Unique_ID_Counter = value;
}

CVMObjectHandle CRuntime_Profile::Get_Skills(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Skill_Array;
}

void CRuntime_Profile::Set_Skills(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Skill_Array = value;
}

CVMObjectHandle CRuntime_Profile::Get_Items(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Item_Array;
}

void CRuntime_Profile::Set_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Item_Array = value;
}

CVMObjectHandle CRuntime_Profile::Get_Applied_DLC_IDs(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->DLC_Array;
}

void CRuntime_Profile::Set_Applied_DLC_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->DLC_Array = value;
}

CVMObjectHandle CRuntime_Profile::Get_Unlocked_Item_IDs(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Unlocked_Item_Array;
}

void CRuntime_Profile::Set_Unlocked_Item_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Unlocked_Item_Array = value;
}

CVMObjectHandle CRuntime_Profile::Get_Unlocked_Skill_IDs(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Unlocked_Skill_Array;
}

void CRuntime_Profile::Set_Unlocked_Skill_IDs(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Unlocked_Skill_Array = value;
}

CVMObjectHandle CRuntime_Profile::Get_Stat_Multipliers(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Stat_Multipliers;
}

void CRuntime_Profile::Set_Stat_Multipliers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Stat_Multipliers = value;
}

int CRuntime_Profile::Has_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	SkillArchetype* skill = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Has_Skill(skill) ? 1 : 0;
}

CVMObjectHandle CRuntime_Profile::Unlock_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	SkillArchetype* skill = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Unlock_Skill(skill)->script_object;
}

CVMObjectHandle CRuntime_Profile::Get_Skill(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	SkillArchetype* skill = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Get_Skill(skill)->script_object;
}

int CRuntime_Profile::Has_Space_For_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Is_Space_For_Item(item) ? 1 : 0;
}

int CRuntime_Profile::Get_Inventory_Space(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return profile->Get_Inventory_Space();
}

int CRuntime_Profile::Has_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Has_Item(item) ? 1 : 0;
}

CVMObjectHandle CRuntime_Profile::Add_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Add_Item(item)->script_object;
}

void CRuntime_Profile::Remove_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle item, int bConsume)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	Item* i = reinterpret_cast<Item*>(item.Get()->Get_Meta_Data());
	profile->Remove_Item(i, bConsume != 0);
}

CVMObjectHandle CRuntime_Profile::Get_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	Item* i = profile->Get_Item(item);
	return i == NULL ? NULL : i->script_object.Get();
}

CVMObjectHandle CRuntime_Profile::Get_Preferred_Consume_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	Item* i = profile->Get_Preferred_Consume_Item(item);
	return i == NULL ? NULL : i->script_object.Get();
}

CVMObjectHandle CRuntime_Profile::Get_Item_Slot(CVirtualMachine* vm, CVMValue self, int slot)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	Item* item = profile->Get_Item_Slot(slot);
	return item == NULL ? NULL : item->script_object.Get();
}

void CRuntime_Profile::Equip_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	Item* item = reinterpret_cast<Item*>(value.Get()->Get_Meta_Data());
	profile->Equip_Item(item);
}

void CRuntime_Profile::Level_Up(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	profile->Level_Up();
}

int CRuntime_Profile::Give_XP(CVirtualMachine* vm, CVMValue self, int amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));	
	return profile->Give_XP(amount) ? 1 : 0;
}

int CRuntime_Profile::Give_Fractional_XP(CVirtualMachine* vm, CVMValue self, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Give_Fractional_XP(amount);
}

void CRuntime_Profile::Combine_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle ia, CVMObjectHandle ib)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Item* item_a = reinterpret_cast<Item*>(ia.Get()->Get_Meta_Data());	
	Item* item_b = reinterpret_cast<Item*>(ib.Get()->Get_Meta_Data());	

	profile->Combine_Item(item_a, item_b);
}

CVMObjectHandle CRuntime_Profile::Get_Attached_Items(CVirtualMachine* vm, CVMValue self, CVMObjectHandle other_item)
{	
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Item* parent = reinterpret_cast<Item*>(other_item.Get()->Get_Meta_Data());

	CVMLinkedSymbol* arr_symbol = vm->Get_Symbol_Table_Entry(vm->Get_Active_Function()->symbol->method_data->return_data_type_class_index);

	CVMObjectHandle result = vm->New_Object(arr_symbol, false);
	CVMObject* obj_result = result.Get();

	std::vector<Item*> items = profile->Get_Attached_Items(parent);
	obj_result->Resize(items.size());

	int index = 0;
	for (std::vector<Item*>::iterator iter = items.begin(); iter != items.end(); iter++, index++)
	{
		obj_result->Get_Slot(index).object_value = (*iter)->script_object;
	}

	return result;
}

CVMObjectHandle CRuntime_Profile::Get_Skill_Slot(CVirtualMachine* vm, CVMValue self, int index)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	Skill* skill = profile->Get_Skill_Slot(index);
	return skill == NULL ? NULL : skill->script_object.Get();
}

void CRuntime_Profile::Combine_Stat_Multipliers(CVirtualMachine* vm, CVMValue self, CVMObjectHandle output_array, CVMObjectHandle skill_multiplier_array, int bAllowHealthRegen, int bAllowAmmoRegen)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));

	for (int i = 0; i < Profile_Stat::COUNT; i++)
	{
		float skill_multiplier = skill_multiplier_array.Get()->Get_Slot(i).float_value;
		float profile_multiplier = profile->Get_Stat_Multiplier((Profile_Stat::Type)i);
		float item_stat_multiplier = profile->Get_Item_Stat_Multiplier((Profile_Stat::Type)i);

		// HACKED: Health regen is only valid if we haven't taken damage for long enough. blah
		if (i == Profile_Stat::Health_Regen && bAllowHealthRegen == false)
		{
			profile_multiplier = 0.0f;
			item_stat_multiplier = 0.0f;
		}

		// HACKED: Ammo regen is only valid if we haven't taken damage for long enough. blah
		if (i == Profile_Stat::Ammo_Regen && bAllowAmmoRegen == false)
		{
			profile_multiplier = 0.0f;
			item_stat_multiplier = 0.0f;
		}

		if (i == Profile_Stat::Health_Regen || 
			i == Profile_Stat::Ammo_Regen || 
			i == Profile_Stat::Wallet_Interest)
		{
			output_array.Get()->Get_Slot(i).float_value = (skill_multiplier + profile_multiplier + item_stat_multiplier);
		}
		else
		{
			output_array.Get()->Get_Slot(i).float_value = (skill_multiplier * profile_multiplier * item_stat_multiplier);
		}
	}
}

void CRuntime_Profile::Add_Unpacked_Fractional_XP(CVirtualMachine* vm, CVMValue self, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Add_Unpacked_Fractional_XP(amount);
}

void CRuntime_Profile::Add_Unpacked_Item(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	profile->Add_Unpacked_Item(item);
}

void CRuntime_Profile::Add_Unpacked_Coins(CVirtualMachine* vm, CVMValue self, int coins)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Add_Unpacked_Coins(coins);
}




CVMObjectHandle CRuntime_Profile::Get_Profile_Item_Stats(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Profile_Item_Stats_Array;
}

void CRuntime_Profile::Set_Profile_Item_Stats(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Profile_Item_Stats_Array = value;
}

float CRuntime_Profile::Get_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Get_Item_Stat(item, (ProfileItemStat::Type)stat);
}

void CRuntime_Profile::Set_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	profile->Set_Item_Stat(item, (ProfileItemStat::Type)stat, amount);
}

void CRuntime_Profile::Increment_Item_Stat(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, int stat, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	ItemArchetype* item = reinterpret_cast<ItemArchetype*>(value.Get()->Get_Meta_Data());
	profile->Increment_Item_Stat(item, (ProfileItemStat::Type)stat, amount);
}




CVMObjectHandle CRuntime_Profile::Get_Profile_Skill_Energies(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Profile_Skill_Energies_Array;
}

void CRuntime_Profile::Set_Profile_Skill_Energies(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Profile_Skill_Energies_Array = value;
}

float CRuntime_Profile::Get_Skill_Energy(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	return profile->Get_Skill_Energy(item);
}

void CRuntime_Profile::Set_Skill_Energy(CVirtualMachine* vm, CVMValue self, CVMObjectHandle value, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	SkillArchetype* item = reinterpret_cast<SkillArchetype*>(value.Get()->Get_Meta_Data());
	profile->Set_Item_Stat(item, amount);
}

void CRuntime_Profile::Increment_Skill_Energies(CVirtualMachine* vm, CVMValue self, int stat, float amount)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Increment_Skill_Energies((SkillEnergyType::Type)stat, amount);
}



int CRuntime_Profile::Get_Is_Male(CVirtualMachine* vm, CVMValue self)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	return profile->Is_Male ? 1 : 0;
}

void CRuntime_Profile::Set_Is_Male(CVirtualMachine* vm, CVMValue self, int value)
{
	Profile* profile = reinterpret_cast<Profile*>(self.object_value.GetNullCheck(vm)->Get_Meta_Data_NullCheck(vm));
	profile->Is_Male = !!value;
}

void CRuntime_Profile::Bind(CVirtualMachine* vm)
{
	vm->Get_Bindings()->Bind_Method<void,float>("Profile", "Add_Unpacked_Fractional_XP", &Add_Unpacked_Fractional_XP);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Add_Unpacked_Item", &Add_Unpacked_Item);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Add_Unpacked_Coins", &Add_Unpacked_Coins);

	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Is_Male", &Get_Is_Male);
	vm->Get_Bindings()->Bind_Method<void, int>("Profile", "Set_Is_Male", &Set_Is_Male);

	vm->Get_Bindings()->Bind_Method<CVMString>("Profile", "Get_Name", &Get_Name);
	vm->Get_Bindings()->Bind_Method<void,CVMString>("Profile", "Set_Name", &Set_Name);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Level", &Get_Level);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Level", &Set_Level);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Level_XP", &Get_Level_XP);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Level_XP", &Set_Level_XP);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Next_Level_XP", &Get_Next_Level_XP);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Coins", &Get_Coins);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Coins", &Set_Coins);
	vm->Get_Bindings()->Bind_Method<float>("Profile", "Get_Coins_Interest", &Get_Coins_Interest);
	vm->Get_Bindings()->Bind_Method<void,float>("Profile", "Set_Coins_Interest", &Set_Coins_Interest);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Skill_Points", &Get_Skill_Points);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Skill_Points", &Set_Skill_Points);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Inventory_Size", &Get_Inventory_Size);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Inventory_Size", &Set_Inventory_Size);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Wallet_Size", &Get_Wallet_Size);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Wallet_Size", &Set_Wallet_Size);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Unique_ID_Counter", &Get_Unique_ID_Counter);
	vm->Get_Bindings()->Bind_Method<void,int>("Profile", "Set_Unique_ID_Counter", &Set_Unique_ID_Counter);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Skills", &Get_Skills);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Set_Skills", &Set_Skills);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Items", &Get_Items);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Set_Items", &Set_Items);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Stat_Multipliers", &Get_Stat_Multipliers);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Set_Stat_Multipliers", &Set_Stat_Multipliers);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle>("Profile", "Has_Skill", &Has_Skill);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Profile", "Unlock_Skill", &Unlock_Skill);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Profile", "Get_Skill", &Get_Skill);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle>("Profile", "Has_Space_For", &Has_Space_For_Item);
	vm->Get_Bindings()->Bind_Method<int>("Profile", "Get_Inventory_Space", &Get_Inventory_Space);
	vm->Get_Bindings()->Bind_Method<int,CVMObjectHandle>("Profile", "Has_Item", &Has_Item);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Profile", "Add_Item", &Add_Item);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,int>("Profile", "Remove_Item", &Remove_Item);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Profile", "Get_Item", &Get_Item);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle, CVMObjectHandle>("Profile", "Get_Preferred_Consume_Item", &Get_Preferred_Consume_Item);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,int>("Profile", "Get_Item_Slot", &Get_Item_Slot);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,int>("Profile", "Get_Skill_Slot", &Get_Skill_Slot);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Equip_Item", &Equip_Item);
	vm->Get_Bindings()->Bind_Method<void>("Profile", "Level_Up", &Level_Up);
	vm->Get_Bindings()->Bind_Method<int,int>("Profile", "Give_XP", &Give_XP);
	vm->Get_Bindings()->Bind_Method<int,float>("Profile", "Give_Fractional_XP", &Give_Fractional_XP);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Applied_DLC_IDs", &Get_Applied_DLC_IDs);
	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle>("Profile", "Set_Applied_DLC_IDs", &Set_Applied_DLC_IDs);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Unlocked_Item_IDs", &Get_Unlocked_Item_IDs);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Profile", "Set_Unlocked_Item_IDs", &Set_Unlocked_Item_IDs);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Unlocked_Skill_IDs", &Get_Unlocked_Skill_IDs);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Profile", "Set_Unlocked_Skill_IDs", &Set_Unlocked_Skill_IDs);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Profile_Item_Stats", &Get_Profile_Item_Stats);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Profile", "Set_Profile_Item_Stats", &Set_Profile_Item_Stats);
	vm->Get_Bindings()->Bind_Method<float, CVMObjectHandle, int>("Profile", "Get_Item_Stat", &Get_Item_Stat);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle, int, float>("Profile", "Set_Item_Stat", &Set_Item_Stat);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle, int, float>("Profile", "Increment_Item_Stat", &Increment_Item_Stat);

	vm->Get_Bindings()->Bind_Method<CVMObjectHandle>("Profile", "Get_Profile_Skill_Energies", &Get_Profile_Skill_Energies);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle>("Profile", "Set_Profile_Skill_Energies", &Set_Profile_Skill_Energies);
	vm->Get_Bindings()->Bind_Method<float, CVMObjectHandle>("Profile", "Get_Skill_Energy", &Get_Skill_Energy);
	vm->Get_Bindings()->Bind_Method<void, CVMObjectHandle, float>("Profile", "Set_Skill_Energy", &Set_Skill_Energy);
	vm->Get_Bindings()->Bind_Method<void, int, float>("Profile", "Increment_Skill_Energies", &Increment_Skill_Energies);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,CVMObjectHandle>("Profile", "Combine_Items", &Combine_Items);
	vm->Get_Bindings()->Bind_Method<CVMObjectHandle,CVMObjectHandle>("Profile", "Get_Attached_Items", &Get_Attached_Items);

	vm->Get_Bindings()->Bind_Method<void,CVMObjectHandle,CVMObjectHandle,int,int>("Profile", "Combine_Stat_Multipliers", &Combine_Stat_Multipliers);
}
