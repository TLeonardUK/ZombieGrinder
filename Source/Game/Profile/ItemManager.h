// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GAME_ITEM_MANAGER_
#define _GAME_ITEM_MANAGER_

#include "Generic/Patterns/Singleton.h"
#include "Generic/Types/Color.h"
#include "Generic/Types/Vector2.h"
#include <vector>

#include "Game/Profile/Profile.h"

#include "Engine/Renderer/Atlases/Atlas.h"

#include "XScript/VirtualMachine/CVMObject.h"
#include "XScript/VirtualMachine/CGarbageCollector.h"

class BinaryStream;
struct CVMLinkedSymbol;
struct ItemArchetype;

#define INVENTORY_ITEM_PLAYTIME_DROP_ID			10
#define INVENTORY_ITEM_ACCESSORIES_DROP_ID		100
#define INVENTORY_ITEM_GEMS_DROP_ID				110
#define INVENTORY_ITEM_HATS_DROP_ID				120
#define INVENTORY_ITEM_PAINTS_DROP_ID			130
#define INVENTORY_ITEM_WEAPONS_DROP_ID			140

#define INVENTORY_ITEM_ACCESSORIES_PROBABILITY	30
#define INVENTORY_ITEM_GEMS_PROBABILITY			10
#define INVENTORY_ITEM_HATS_PROBABILITY			20
#define INVENTORY_ITEM_PAINTS_PROBABILITY		20
#define INVENTORY_ITEM_WEAPONS_PROBABILITY		30

#define INVENTORY_ITEM_SELL_PRICE_MULTIPLIER	3.0

struct ItemCombineMethod
{
	enum Type
	{
		Tint	= 0,		// Changes item color
		Attach	= 1,		// Attachs item as a "sub-item" of the destination item.
		Merge	= 2,		// Merges the item and produces a new item of Result_Item type.
		Repair	= 3			// Repairs durability.
	};
};

struct ItemSlot
{
	enum Type
	{
		Head		= 0,
		Accessory	= 1,
		Weapon		= 2,
		All			= 3,

		COUNT		
	};
};

struct ItemRarity
{
	enum Type
	{
		Common		= 0,
		Uncommon	= 1,
		Rare		= 2,
		Ultra_Rare	= 3,
		Legendary	= 4
	};
};

struct ItemUnlockCriteria
{
	enum Type
	{
		Required_Rank				= 0,
		Kills_With_Weapon			= 1,
		Damage_With_Weapon			= 2,
		Ailments_From_Weapon		= 3
	};
};


struct ItemCombination
{
public:
	std::string		other_group;
	int				combine_method;
	std::string		result_item;
};

struct Item;

struct UniqueItemStats
{
	float Stats[Profile_Stat::COUNT];

	UniqueItemStats(Item* item);
	UniqueItemStats(u64 seed, bool bEquippable);

	void CalculateBySeed(int seed, bool bEquippable);
	std::string ToString();
};

struct ItemUpgradeTreeNodeModifierType
{
	enum Type
	{
		// Keep in sync with Particle_Multiplier_Type for simplicity.

		// Base states.
		Damage,
		AmmoCapacity,
		RateOfFire,
		ReloadSpeed,
		ProjectileSpeed,
		Accuracy,
		Penetrating,
		Durability,

		// Specials
		TriBarrel,
		QuadBarrel,
		MultiTap,
		Suicidal,

		COUNT
	};
};

struct ItemUpgradeTreeNodeModifier
{
public:
	CVMGCRoot script_object;

	ItemUpgradeTreeNodeModifierType::Type modifier;
	float scale;

	ItemUpgradeTreeNodeModifier()
		: scale(1.0f)
		, modifier(ItemUpgradeTreeNodeModifierType::Damage)
	{
	}

	std::string To_Description_String()
	{
		std::string value_text = StringHelper::Format_Number(scale * 100.0f, true);

		switch (modifier)
		{
		// Base states.
		case ItemUpgradeTreeNodeModifierType::Damage:			return SF("#weapon_upgrade_mod_damage_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::AmmoCapacity:		return SF("#weapon_upgrade_mod_ammocapacity_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::RateOfFire:		return SF("#weapon_upgrade_mod_rateoffire_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::ReloadSpeed:		return SF("#weapon_upgrade_mod_reloadspeed_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::ProjectileSpeed:	return SF("#weapon_upgrade_mod_projectilespeed_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Accuracy:			return SF("#weapon_upgrade_mod_accuracy_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Penetrating:		return SF("#weapon_upgrade_mod_penetration_description", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Durability:		return SF("#weapon_upgrade_mod_durability_description", value_text.c_str());

		// Specials
		case ItemUpgradeTreeNodeModifierType::TriBarrel:		return S("#weapon_upgrade_mod_tri_barrel_description");
		case ItemUpgradeTreeNodeModifierType::QuadBarrel:		return S("#weapon_upgrade_mod_quad_barrel_description");
		case ItemUpgradeTreeNodeModifierType::MultiTap:			return S("#weapon_upgrade_mod_multi_tap_description");
		case ItemUpgradeTreeNodeModifierType::Suicidal:			return S("#weapon_upgrade_mod_suicidal_description");
		}

		return "";
	}

	std::string To_BBCode_Description_String()
	{
		std::string value_text = StringHelper::Format_Number(scale * 100.0f, true);

		switch (modifier)
		{
			// Base states.
		case ItemUpgradeTreeNodeModifierType::Damage:			return SF("#weapon_upgrade_mod_damage_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::AmmoCapacity:		return SF("#weapon_upgrade_mod_ammocapacity_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::RateOfFire:		return SF("#weapon_upgrade_mod_rateoffire_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::ReloadSpeed:		return SF("#weapon_upgrade_mod_reloadspeed_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::ProjectileSpeed:	return SF("#weapon_upgrade_mod_projectilespeed_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Accuracy:			return SF("#weapon_upgrade_mod_accuracy_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Penetrating:		return SF("#weapon_upgrade_mod_penetration_description_bbcode", value_text.c_str());
		case ItemUpgradeTreeNodeModifierType::Durability:		return SF("#weapon_upgrade_mod_durability_description_bbcode", value_text.c_str());

			// Specials
		case ItemUpgradeTreeNodeModifierType::TriBarrel:		return S("#weapon_upgrade_mod_tri_barrel_description_bbcode");
		case ItemUpgradeTreeNodeModifierType::QuadBarrel:		return S("#weapon_upgrade_mod_quad_barrel_description_bbcode");
		case ItemUpgradeTreeNodeModifierType::MultiTap:			return S("#weapon_upgrade_mod_multi_tap_description_bbcode");
		case ItemUpgradeTreeNodeModifierType::Suicidal:			return S("#weapon_upgrade_mod_suicidal_description_bbcode");
		}

		return "";
	}
};

struct ItemUpgradeTreeNode
{
public:
	CVMGCRoot script_object;
	CVMGCRoot modifiers;

	CVMLinkedSymbol* gem_type;

	int x;
	int y;

	std::string name;
	int id;

	int cost;

	bool bConnectUp;
	bool bConnectDown;
	bool bConnectLeft;
	bool bConnectRight;

	void Get_Modifiers(std::vector<ItemUpgradeTreeNodeModifier*>& output)
	{
		CVMObject* obj = modifiers.Get().Get();
		for (int i = 0; i < obj->Slot_Count(); i++)
		{
			output.push_back(static_cast<ItemUpgradeTreeNodeModifier*>(obj->Get_Slot(i).object_value.Get()->Get_Meta_Data()));
		}
	}

	ItemUpgradeTreeNode()
		: script_object(NULL)
		, modifiers(NULL)
		, bConnectUp(false)
		, bConnectDown(false)
		, bConnectLeft(false)
		, bConnectRight(false)
		, x(0)
		, y(0)
		, gem_type(NULL)
		, name("")
		, cost(0)
	{
	}
};

struct ItemUpgradeTree
{
public:
	CVMGCRoot script_object;
	CVMGCRoot nodes;

	void Get_Nodes(std::vector<ItemUpgradeTreeNode*>& output)
	{
		CVMObject* obj = nodes.Get().Get();
		for (int i = 0; i < obj->Slot_Count(); i++)
		{
			output.push_back(static_cast<ItemUpgradeTreeNode*>(obj->Get_Slot(i).object_value.Get()->Get_Meta_Data()));
		}
	}

	ItemUpgradeTreeNode* Get_Node(int id)
	{
		CVMObject* obj = nodes.Get().Get();
		for (int i = 0; i < obj->Slot_Count(); i++)
		{
			ItemUpgradeTreeNode* node = static_cast<ItemUpgradeTreeNode*>(obj->Get_Slot(i).object_value.Get()->Get_Meta_Data());
			if (node->id == id)
			{
				return node;
			}
		}
		return NULL;
	}

	void Get_Active_Compound_Modifiers(std::vector<ItemUpgradeTreeNodeModifier>& modifiers, std::vector<int> upgrade_ids)
	{
		std::vector<ItemUpgradeTreeNode*> nodes;
		Get_Nodes(nodes);

		for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
		{
			ItemUpgradeTreeNode* node = *iter;
			if (std::find(upgrade_ids.begin(), upgrade_ids.end(), node->id) != upgrade_ids.end())
			{
				std::vector<ItemUpgradeTreeNodeModifier*> mods;
				node->Get_Modifiers(mods);

				for (std::vector<ItemUpgradeTreeNodeModifier*>::iterator modIter = mods.begin(); modIter != mods.end(); modIter++)
				{
					ItemUpgradeTreeNodeModifier* mod = *modIter;

					bool bFound = false;

					for (std::vector<ItemUpgradeTreeNodeModifier>::iterator resultIter = modifiers.begin(); resultIter != modifiers.end(); resultIter++)
					{
						ItemUpgradeTreeNodeModifier& resultMod = *resultIter;
						if (resultMod.modifier == mod->modifier)
						{
							resultMod.scale += mod->scale;
							bFound = true;
							break;
						}
					}

					if (!bFound)
					{
						ItemUpgradeTreeNodeModifier resultMod;
						resultMod.scale = mod->scale;
						resultMod.modifier = mod->modifier;
						modifiers.push_back(resultMod);
					}
				}
			}
		}
	}

	ItemUpgradeTree()
		: script_object(NULL)
		, nodes(NULL)
	{
	}

};

struct Item
{
public:
	ItemArchetype*	archetype;
	int				unique_id;
	int				recieve_time;
	Color			primary_color;
	bool			was_inventory_drop;
	std::string		inventory_original_drop_id;
	std::string		inventory_drop_id;
	int				equip_slot;
	int				equip_time;
	int				attached_to_id;
	CVMGCRoot		script_object;
	CVMGCRoot		item_upgrade_ids;
	UniqueItemStats	cached_item_stats;
	bool			item_stats_cached;

	float			equip_duration;
	bool			indestructable;

	void Get_Upgrade_Ids(std::vector<int>& output);

	void Add_Upgrade_Id(int id)
	{
		CVMObject* obj = item_upgrade_ids.Get().Get();
		obj->Add_Last(id);
	}

	void Fully_Upgrade();

	int Get_Level();

	std::string Get_Display_Name();

	bool Is_Broken();

	Item()
		: item_stats_cached(false)
		, cached_item_stats(0, false)
		, equip_duration(0.0f)
		, indestructable(false)
		, item_upgrade_ids(NULL)
	{

	}
};

struct ItemArchetype
{
public:
	std::string						varient_prefix;
	std::string						base_name;
	std::string						base_description;
	std::string						category;

	int								item_slot;
	int								cost;

	bool							is_unpackable;
	bool							is_stackable;
	bool							is_buyable;
	bool							is_tintable;
	bool							is_sellable;
	bool							is_tradable;
	bool							is_dropable;
	bool							is_pvp_usable;
	bool							is_equippable;
	bool							is_combinable;
	bool							is_inventory_droppable;
	bool							is_premium_only;
	std::string						premium_price;

	ItemUnlockCriteria::Type		unlock_critera;
	CVMLinkedSymbol*				unlock_critera_item;
	int								unlock_critera_threshold;

	int								max_stack;
	ItemRarity::Type				rarity;

	Color							default_tint;

	std::string						icon_animation_name;
	AtlasAnimation*					icon_animations[8];
	AtlasAnimation*					icon_tint_animations[8];
	Vector2							icon_offset;
	bool							is_icon_direction_based;
	bool							is_icon_overlay;
	
	std::string						body_animation_name;
	AtlasAnimation*					body_animations[8];
	AtlasAnimation*					body_tint_animations[8];
		
	std::string						combine_group;
	std::string						combine_name;
	ItemCombination					combinations[8];	
	int								combination_count;
	CVMGCRoot						combinations_array;
	bool							use_custom_color;
	
	CVMLinkedSymbol*				weapon_type;
	CVMLinkedSymbol*				ammo_type;
	CVMLinkedSymbol*				post_process_fx_type;

	CVMGCRoot						upgrade_tree;

	CVMGCRoot						script_object;

	float							max_durability;

	float							initial_armour_amount;

	std::string						override_inventory_id_name;

	// Temporary variables that should not fucking be in here
	// but should be encapsulated elsewhere. Rush job!
	int								tmp_unlocked;

	ItemArchetype()
		: max_durability(2 * 60 * 60.0f) // 2 hours of activly playing. 
		, is_premium_only(false)
		, premium_price("")
		, use_custom_color(false)
		, unlock_critera(ItemUnlockCriteria::Required_Rank)
		, unlock_critera_item(NULL)
		, unlock_critera_threshold(0)
		, initial_armour_amount(0.0f)
		, upgrade_tree(NULL)
		, varient_prefix("")
		, override_inventory_id_name("")
	{
	}

	ItemUpgradeTree* Get_Upgrade_Tree()
	{
		return upgrade_tree.Get().Get() == NULL ? NULL : static_cast<ItemUpgradeTree*>(upgrade_tree.Get().Get()->Get_Meta_Data());
	}

	std::string Get_Display_Name(bool indestructable, int level, bool no_rarity = false)
	{
		std::string name = Get_Base_Display_Name(no_rarity);
		if (item_slot != ItemSlot::All || (combine_group == "Paints" && indestructable))
		{
			name = Get_Fragile_Display_Name(no_rarity);
			if (indestructable)
			{
				name = Get_Indestructable_Display_Name(no_rarity);
			}
		}

		if (level > 0 && !indestructable)
		{
			name = StringHelper::Format("%s +%i", name.c_str(), level);
		}

		return name;
	}

	int Get_Inventory_ItemDefID();
	int Get_Not_Overriden_Inventory_ItemDefID();
	std::string Get_Indestructable_Display_Name(bool no_rarity = false);
	std::string Get_Fragile_Display_Name(bool no_rarity = false);
	std::string Get_Base_Display_Name(bool no_rarity = false);

	std::string Get_Description(bool is_indestructable);
};

class ItemManager : public Singleton<ItemManager>
{
	MEMORY_ALLOCATOR(ItemManager, "Game");

private:
	bool m_init;
	CVMLinkedSymbol* m_item_class;
	CVMLinkedSymbol* m_item_archetype_class;
	CVMLinkedSymbol* m_item_upgrade_tree_class;
	std::vector<ItemArchetype> m_archetypes;

	std::vector<ItemUpgradeTree> m_upgrade_trees;

public:
	ItemManager();
	~ItemManager();

	ItemArchetype* Find_Archetype(const char* name);
	ItemArchetype* Find_Archetype(int id);
	ItemArchetype* Find_Archetype(CVMLinkedSymbol* symbol);
	ItemArchetype* Find_Archetype_By_Combo_Name(const char* name);
	ItemArchetype* Find_Archetype_By_InventoryId(u64 id);

	ItemUpgradeTree* Find_Upgrade_Tree(CVMLinkedSymbol* symbol);

	std::vector<ItemArchetype*> Get_Archetypes();

	void Dump_Item_Schema_Generator(std::string& schema, std::string type, int base_id);
	void Dump_Item_Schema();

	bool Init();

};

#endif

