// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_PROFILE_USERPROFILE_
#define _GAME_PROFILE_USERPROFILE_

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

#include "Game/Profile/SkillManager.h"

#include "Engine/IO/BinaryStream.h"

#include "Engine/Engine/FrameTime.h"

class ProfileManager;
struct ItemArchetype;
struct SkillArchetype;
struct Skill;
struct Item;
struct DLC;

struct ProfileItemStack
{
	ItemArchetype*		Archetype;
	std::vector<Item*>	Items;

	int category_id;
	int name_id;

	static bool Sort_Predicate(const ProfileItemStack& a, const ProfileItemStack& b);
};

struct ProfileStatRange
{
	float default_value;
	float range_min;
	float range_max;
	float negative_range_min;
	float negative_range_max;

	ProfileStatRange(float InDefault, float InMin, float InMax, float InNegMin, float InNegMax)
		: default_value(InDefault) 
		, range_min(InMin)
		, range_max(InMax)
		, negative_range_min(InNegMin)
		, negative_range_max(InNegMax)
	{
	}
};

struct Profile_Stat
{
	enum Type
	{
		Wallet_Interest,

		Max_Health,
		Health_Regen,
		Speed,
		Ammo_Capacity,
		Ammo_Regen,
		Gold_Drop,
		Gold_Magnet,
		Buy_Price,
		Sell_Price,
		Gem_Drop,
		Gem_Magnet,
		XP,

		Melee_Damage,
		Melee_Stamina,

		Rifle_Damage,
		Rifle_Ammo,

		Heavy_Weapon_Damage,
		Heavy_Weapon_Ammo,
		Explosion_Radius,
		Enemy_Status_Duration,

		Turret_Damage,
		Turret_Ammo,

		Small_Arms_Damage,
		Small_Arms_Ammo,

		Buff_Grenade_Radius,
		Buff_Grenade_Ammo,

		Ghost_Life_Time,
		Ghost_Revive_Time,
		Healing_Speed,

		// Generic multipliers
		Damage,
		Damage_Taken,
		Rate_Of_Fire,
		Reload_Speed,
		Ammo_Usage,
		Targetable,

		COUNT
	};

	static ProfileStatRange Ranges[COUNT];
	static const char* Names[COUNT];
};

struct ProfileUnpackEventType
{
	enum Type
	{
		Item,
		Coins,
		XP
	};
};

struct ProfileUnpackEvent
{
	ProfileUnpackEventType::Type type;
	ItemArchetype* item;
	int amount;
};

struct ProfileItemStat
{
	enum Type
	{
		Ailments_Inflicted,
		Damage_Inflicted,
		Enemies_Killed,
		Shots_Fired
	};
};

struct Profile
{
	MEMORY_ALLOCATOR(Profile, "Game");

public:
	Profile();

public:
	enum
	{
		max_name_length = 24,
	};
	
	CVMGCRoot							Script_Object;

	std::string							Name;
	int									Level;			// We store level and xp so we 
	int									Level_XP;		// don't fuck up peoples progression if we change the xp-curve post-release.

	int									Coins;
	float								Coins_Interest;
	int									Skill_Points;
	int									Inventory_Size;
	int									Wallet_Size;
	
	bool								Is_Male;

	int									Inventory_Space;
	bool								Inventory_Space_Dirty;

	bool								Item_Slots_Dirty;
	Item*								Item_Slot_Items[99];

	CVMGCRoot							Stat_Multipliers;
	CVMGCRoot							Skill_Array;
	CVMGCRoot							Item_Array;
	CVMGCRoot							DLC_Array;
	CVMGCRoot							Unlocked_Item_Array;
	CVMGCRoot							Unlocked_Skill_Array;
	CVMGCRoot							Profile_Item_Stats_Array;
	CVMGCRoot							Profile_Skill_Energies_Array;

	CVMLinkedSymbol*					Profile_Item_Stat_Class;
	CVMLinkedSymbol*					Profile_Skill_Energy_Class;
			
	int									Unique_ID_Counter;

	std::vector<ProfileUnpackEvent>		Unpack_Events;
	std::vector<ItemArchetype*>			Item_Unlock_Events;
	std::vector<SkillArchetype*>		Skill_Unlock_Events;

private:
	bool Can_Combo_Create_Internal(ItemArchetype* result_item, std::string result_item_group, std::vector<ItemArchetype*>& resources);

public:
	float Get_Level_Progress();

	int Get_Item_Count(ItemArchetype* type);

	Item* Combo_Create(ItemArchetype* archetype);
	bool Can_Combo_Create(ItemArchetype* archetype);

	int Get_Inventory_Space();

	int Get_Total_Items();
	int Get_Inventory_Size();
	std::vector<Item*> Get_Items();
	std::vector<ProfileItemStack> Get_Item_Stacks();
	bool Stack_Space_Available(ItemArchetype* type);
	bool Is_Space_For_Item(ItemArchetype* type);
	Item* Add_Item(ItemArchetype* type);
	void Remove_Item(Item* item, bool bConsume = false);	
	void Unequip_Item(Item* skill);
	void Equip_Item(Item* skill);
	void Ensure_Valid_Equips(Item* ignore_item = NULL);
	Item* Get_Item_Slot(int slot);
	std::vector<Item*> Get_Attached_Items(Item* item);
	bool Can_Combine_Item(Item* item_a, Item* item_b);
	void Combine_Item(Item* item_a, Item* item_b, Color custom_tint_color = Color::White);
	Item* Get_Item(ItemArchetype* type);
	Item* Get_Preferred_Consume_Item(ItemArchetype* type);
	bool Has_Item(ItemArchetype* type);
	void Level_Up();
	int Give_XP(int amount);
	int Give_Fractional_XP(float amount);

	void Add_Unpacked_Coins(int coins);
	void Add_Unpacked_Item(ItemArchetype* type);
	void Add_Unpacked_Fractional_XP(float amount);

	int Get_Free_Slots();

	void Unpack_Item(Item* item);

	Skill* Unlock_Skill(SkillArchetype* archetype);
	bool Has_Skill(SkillArchetype* archetype);
	Skill* Get_Skill(SkillArchetype* archetype);
	void Unequip_Skill(Skill* skill);
	void Equip_Skill(Skill* skill, int slot);
	Skill* Get_Longest_Equipped_Skill();
	Skill* Get_Skill_Slot(int slot);
	int Get_Next_Equip_Slot();
	int Get_Total_Used_Skill_Points();
	void Clear_Skills();

	float Get_Stat_Multiplier(Profile_Stat::Type stat);
	void Set_Stat_Multiplier(Profile_Stat::Type stat, float value);

	float Get_Item_Stat_Multiplier(Profile_Stat::Type stat);

	int Get_Reroll_Cost();
	void Reroll_Skills();

	bool Has_DLC(DLC* dlc);
	void Apply_DLC(DLC* dlc);

	void Check_Unlocks();
	bool Is_Item_Unlocked(ItemArchetype* item);
	bool Is_Skill_Unlocked(SkillArchetype* item);

	int Get_Unique_ID();

	float Get_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat);
	void Set_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat, float amount);
	void Increment_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat, float amount);

	float Get_Skill_Energy(SkillArchetype* archetype);
	void Set_Item_Stat(SkillArchetype* archetype, float amount);
	void Increment_Skill_Energies(SkillEnergyType::Type type, float amount);

	void Sanitize();

	void Tick(const FrameTime& time);

};

#endif

