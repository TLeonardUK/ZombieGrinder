// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Profile/ItemManager.h"
#include "XScript/VirtualMachine/CVirtualMachine.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "Engine/Online/OnlineInventory.h"

#include "Engine/Engine/EngineOptions.h"
#include "Game/Runner/GameOptions.h"

#include "Engine/Resources/ResourceFactory.h"

#include "Engine/IO/StreamFactory.h"

#include "Engine/Renderer/Textures/Pixelmap.h"
#include "Engine/Renderer/Textures/PixelmapFactory.h"

#include "Engine/Localise/Locale.h"

#include "Game/Scene/Actors/ScriptedActor.h"

#include "Game/Runner/Game.h"

UniqueItemStats::UniqueItemStats(Item* item)
{
	CalculateBySeed((int)StringHelper::To_U64(item->inventory_original_drop_id), item->archetype->is_equippable);
}

UniqueItemStats::UniqueItemStats(u64 seed, bool bEquippable)
{
	CalculateBySeed((int)seed, bEquippable);
}

void UniqueItemStats::CalculateBySeed(int seed, bool bEquippable)
{
	// Randomise based on inventory-drop-id.
	Random rnd(seed); // TODO: Change for "original-item" id
	
	// Weights for number of stats to apply to weapon.
	std::vector<RandomWeight> stat_count_weights;
	stat_count_weights.push_back(RandomWeight(1, 60));
	stat_count_weights.push_back(RandomWeight(2, 30));
	stat_count_weights.push_back(RandomWeight(3, 10));

	// Weights for which stats to apply to weapon.
	std::vector<RandomWeight> stat_weights;
	stat_weights.push_back(RandomWeight(Profile_Stat::Health_Regen,				5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Ammo_Regen,				5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Ammo_Capacity,			20));
	stat_weights.push_back(RandomWeight(Profile_Stat::Gold_Drop,				5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Gold_Magnet,				10));
	stat_weights.push_back(RandomWeight(Profile_Stat::Gem_Drop,					5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Gem_Magnet,				10));
	stat_weights.push_back(RandomWeight(Profile_Stat::XP,						5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Damage,					20));
	stat_weights.push_back(RandomWeight(Profile_Stat::Rate_Of_Fire,				20));
	stat_weights.push_back(RandomWeight(Profile_Stat::Reload_Speed,				20));
	stat_weights.push_back(RandomWeight(Profile_Stat::Melee_Stamina,			5));
	stat_weights.push_back(RandomWeight(Profile_Stat::Healing_Speed,			10));
	stat_weights.push_back(RandomWeight(Profile_Stat::Buff_Grenade_Radius,		10));
	stat_weights.push_back(RandomWeight(Profile_Stat::Explosion_Radius,			10));
	stat_weights.push_back(RandomWeight(Profile_Stat::Enemy_Status_Duration,	10));

	// Weights for stat strengths
	std::vector<RandomWeight> stat_strength_weights;
	stat_strength_weights.push_back(RandomWeight(1, 10));
	stat_strength_weights.push_back(RandomWeight(2, 10));
	stat_strength_weights.push_back(RandomWeight(3, 10));
	stat_strength_weights.push_back(RandomWeight(4, 10));
	stat_strength_weights.push_back(RandomWeight(5, 10)); // 50%
	stat_strength_weights.push_back(RandomWeight(6, 5));
	stat_strength_weights.push_back(RandomWeight(7, 5));
	stat_strength_weights.push_back(RandomWeight(8, 5));
	stat_strength_weights.push_back(RandomWeight(9, 5));
	stat_strength_weights.push_back(RandomWeight(10, 5));
	stat_strength_weights.push_back(RandomWeight(11, 5)); // 80%
	stat_strength_weights.push_back(RandomWeight(12, 2));  
	stat_strength_weights.push_back(RandomWeight(13, 2));
	stat_strength_weights.push_back(RandomWeight(14, 2));
	stat_strength_weights.push_back(RandomWeight(15, 2));
	stat_strength_weights.push_back(RandomWeight(16, 2)); // 90%
	stat_strength_weights.push_back(RandomWeight(17, 2));
	stat_strength_weights.push_back(RandomWeight(18, 1));
	stat_strength_weights.push_back(RandomWeight(19, 1));
	stat_strength_weights.push_back(RandomWeight(20, 1));
	stat_strength_weights.push_back(RandomWeight(21, 1));
	stat_strength_weights.push_back(RandomWeight(22, 1));
	stat_strength_weights.push_back(RandomWeight(23, 1));
	stat_strength_weights.push_back(RandomWeight(24, 1));
	stat_strength_weights.push_back(RandomWeight(25, 1)); // 100%

	for (int i = 0; i < Profile_Stat::COUNT; i++)
	{
		Stats[i] = Profile_Stat::Ranges[i].default_value;
	}

	// Only equippable items have stats.
	if (!bEquippable)
	{
		return;
	}

	int count = rnd.Next_Weighted(stat_count_weights);

	// DISABLED
	// We don't want to use this for the time being.
	count = 0;
	// DISABLED

	for (int i = 0; i < count; i++)
	{
		int stat = rnd.Next_Weighted(stat_weights);

		// Select strength of the stat.
		float strength = rnd.Next_Weighted(stat_strength_weights) / 100.0f;

		// Work out the actual scalar value the stat requires based on its range.
		float range_min = Profile_Stat::Ranges[stat].range_min;
		float range_max = Profile_Stat::Ranges[stat].range_max;
		float negative_range_min = Profile_Stat::Ranges[stat].negative_range_min;
		float negative_range_max = Profile_Stat::Ranges[stat].negative_range_max;
		float scalar = range_min + ((range_max - range_min) * strength);

		// Negative scalars.
		if (i != 0) // First value is always positive, we don't want to end up with items with only negative properties.
		{
			if (rnd.Next(0, 3) == 0)
			{
				scalar = negative_range_max - ((negative_range_max - negative_range_min) * strength);
			}
		}

		Stats[stat] = scalar;

		//DBG_LOG("[Item %s] stat=%i strength=%f scalar=%f", item->archetype->base_name.c_str(), stat, strength, scalar);
	}
}

std::string UniqueItemStats::ToString()
{
	std::string result = "";

	for (int i = 0; i < Profile_Stat::COUNT; i++)
	{
		float scalar = Stats[i];
		if (scalar != Profile_Stat::Ranges[i].default_value)
		{
			float default_value = Profile_Stat::Ranges[i].default_value;
			float range_min = Profile_Stat::Ranges[i].range_min;
			float range_max = Profile_Stat::Ranges[i].range_max;
			float negative_range_min = Profile_Stat::Ranges[i].negative_range_min;
			float negative_range_max = Profile_Stat::Ranges[i].negative_range_max;

			float diff_sgn = (scalar - default_value) < 0.0f ? -1.0f : 1.0f;
			float pos_sgn = (range_max - default_value) < 0.0f ? -1.0f : 1.0f;

			if (result != "")
			{
				result += "\n";
			}

			float percent_diff = (scalar - default_value) * 100;
			if (percent_diff < 0.0f)
			{
				percent_diff = floorf(percent_diff);
			}
			else
			{
				percent_diff = ceilf(percent_diff);
			}

			std::string percent_value = "";
			bool show_positive = (diff_sgn == pos_sgn);

			if (percent_diff < 0.0f)
			{
				percent_value = StringHelper::Format("%.0f", percent_diff);
			}
			else
			{
				percent_value = StringHelper::Format("+%.0f", percent_diff);
			}

			if (show_positive)
			{
				result += SF("#item_stat_positive", percent_value.c_str(), S(Profile_Stat::Names[i]));
			}
			else
			{
				result += SF("#item_stat_negative", percent_value.c_str(), S(Profile_Stat::Names[i]));
			}
		}
	}

	return result;
}

int ItemArchetype::Get_Inventory_ItemDefID()
{
	int name_hash = 0;
	if (override_inventory_id_name != "")
	{
		name_hash = StringHelper::Hash(override_inventory_id_name.c_str());
	}
	else
	{
		name_hash = script_object.Get().Get()->Get_Symbol()->name_hash;
	}
	return abs(name_hash) % 1000000;
}

int ItemArchetype::Get_Not_Overriden_Inventory_ItemDefID()
{
	int name_hash = name_hash = script_object.Get().Get()->Get_Symbol()->name_hash;
	return abs(name_hash) % 1000000;
}

std::string ItemArchetype::Get_Indestructable_Display_Name(bool no_rarity)
{
	const char* prefix = "#item_indestructable_prefix";
	if (this->combine_group == "Paints")
	{
		prefix = "#item_unlimited_prefix";
	}

	std::string result = S(base_name);

	if (varient_prefix != "")
	{
		result = SF(varient_prefix.c_str(), result.c_str());
	}

	if (!no_rarity)
	{
		switch (rarity)
		{
		case ItemRarity::Common:		result = SF("#item_rarity_common", SF(prefix, result.c_str()));		break;
		case ItemRarity::Uncommon:		result = SF("#item_rarity_uncommon", SF(prefix, result.c_str()));	break;
		case ItemRarity::Rare:			result = SF("#item_rarity_rare", SF(prefix, result.c_str()));		break;
		case ItemRarity::Ultra_Rare:	result = SF("#item_rarity_ultra_rare", SF(prefix, result.c_str()));	break;
		case ItemRarity::Legendary:		result = SF("#item_rarity_legendary", SF(prefix, result.c_str()));	break;
		}
	}
	else
	{
		result = SF(prefix, result.c_str());
	}
	return result;
}

std::string ItemArchetype::Get_Base_Display_Name(bool no_rarity)
{
	std::string result = S(base_name);

	if (varient_prefix != "")
	{
		result = SF(varient_prefix.c_str(), result.c_str());
	}

	if (!no_rarity)
	{
		switch (rarity)
		{
		case ItemRarity::Common:		result = SF("#item_rarity_common", result.c_str());		break;
		case ItemRarity::Uncommon:		result = SF("#item_rarity_uncommon", result.c_str());	break;
		case ItemRarity::Rare:			result = SF("#item_rarity_rare", result.c_str());		break;
		case ItemRarity::Ultra_Rare:	result = SF("#item_rarity_ultra_rare", result.c_str());	break;
		case ItemRarity::Legendary:		result = SF("#item_rarity_legendary", result.c_str());	break;
		}
	}
	return result;
}

std::string ItemArchetype::Get_Fragile_Display_Name(bool no_rarity)
{
	std::string result = S(base_name);

	if (varient_prefix != "")
	{
		result = SF(varient_prefix.c_str(), result.c_str());
	}

	if (!no_rarity)
	{
		switch (rarity)
		{
		case ItemRarity::Common:		result = SF("#item_rarity_common", SF("#item_fragile_prefix", result.c_str()));		break;
		case ItemRarity::Uncommon:		result = SF("#item_rarity_uncommon", SF("#item_fragile_prefix", result.c_str()));	break;
		case ItemRarity::Rare:			result = SF("#item_rarity_rare", SF("#item_fragile_prefix", result.c_str()));		break;
		case ItemRarity::Ultra_Rare:	result = SF("#item_rarity_ultra_rare", SF("#item_fragile_prefix", result.c_str()));	break;
		case ItemRarity::Legendary:		result = SF("#item_rarity_legendary", SF("#item_fragile_prefix", result.c_str()));	break;
		}
	}
	else
	{
		result = SF("#item_fragile_prefix", result.c_str());
	}
	return result;
}

std::string ItemArchetype::Get_Description(bool is_indestructable)
{
	std::string result = S(base_description);

	if (combine_group == "Gems")
	{
		result = SF("#item_single_use_postfix", result.c_str());	
	}
	else if (combine_group == "Paints")
	{
		if (is_indestructable)
		{
			result = SF("#item_unlimited_use_postfix", result.c_str());
		}
		else
		{
			result = SF("#item_single_use_postfix", result.c_str());
		}
	}

	return result;
}

int Item::Get_Level()
{
	if (archetype->Get_Upgrade_Tree() == NULL)
	{
		return 0;
	}

	std::vector<int> upgrade_ids;
	Get_Upgrade_Ids(upgrade_ids);

	std::vector<ItemUpgradeTreeNode*> upgrade_nodes;
	archetype->Get_Upgrade_Tree()->Get_Nodes(upgrade_nodes);

	int current_level = 0;

	for (std::vector<int>::iterator iter = upgrade_ids.begin(); iter != upgrade_ids.end(); iter++)
	{
		int id = *iter;
		ItemUpgradeTreeNode* node = archetype->Get_Upgrade_Tree()->Get_Node(id);
		if (node)
		{
			if (node->cost > 0)
			{
				current_level += node->cost;
			}
		}
	}

	return current_level;
}

std::string Item::Get_Display_Name()
{
	return archetype->Get_Display_Name(indestructable, Get_Level());
}

void Item::Fully_Upgrade()
{
	ItemUpgradeTree* tree = archetype->Get_Upgrade_Tree();
	if (tree)
	{
		std::vector<ItemUpgradeTreeNode*> nodes;
		tree->Get_Nodes(nodes);

		for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
		{
			Add_Upgrade_Id((*iter)->id);
		}
	}
}

bool Item::Is_Broken()
{
	ItemUpgradeTree* tree = archetype->Get_Upgrade_Tree();
	float durability_multiplier = 1.0f;
	if (tree != NULL)
	{
		std::vector<int> upgrade_ids;
		std::vector<ItemUpgradeTreeNodeModifier> modifiers;
		Get_Upgrade_Ids(upgrade_ids);

		tree->Get_Active_Compound_Modifiers(modifiers, upgrade_ids);

		for (std::vector<ItemUpgradeTreeNodeModifier>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
		{
			ItemUpgradeTreeNodeModifier& mod = *iter;
			if (mod.modifier == ItemUpgradeTreeNodeModifierType::Durability)
			{
				durability_multiplier += mod.scale;
			}
		}
	}
	return !indestructable && equip_duration > (archetype->max_durability * durability_multiplier) && archetype->max_durability >= 0.0f;
}


void Item::Get_Upgrade_Ids(std::vector<int>& output)
{
	CVMObject* obj = item_upgrade_ids.Get().Get();

	// If indestructable and no upgrade-id's, add everything from the upgrade tree.
	if (indestructable && obj->Slot_Count() == 0)
	{
		std::vector<ItemUpgradeTreeNode*> nodes;
		ItemUpgradeTree* tree = archetype->Get_Upgrade_Tree();
		
		if (tree)
		{
			tree->Get_Nodes(nodes);

			for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
			{
				ItemUpgradeTreeNode* node = *iter;
				Add_Upgrade_Id(node->id);
			}
		}
	}

	for (int i = 0; i < obj->Slot_Count(); i++)
	{
		output.push_back(obj->Get_Slot(i).int_value);
	}
}

ItemManager::ItemManager()
	: m_init(false)
{
}

ItemManager::~ItemManager()
{
}

ItemArchetype* ItemManager::Find_Archetype(const char* name)
{
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (strcmp(item.script_object.Get().Get()->Get_Symbol()->symbol->name, name) == 0)
		{
			return &item;
		}
	}
	return NULL;
}
ItemArchetype* ItemManager::Find_Archetype_By_InventoryId(u64 id)
{	
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (item.Get_Inventory_ItemDefID() == id)
		{
			return &item;
		}
	}
	return NULL;
}

ItemArchetype* ItemManager::Find_Archetype_By_Combo_Name(const char* name)
{
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (item.combine_name == name)
		{
			return &item;
		}
	}
	return NULL;
}

ItemArchetype* ItemManager::Find_Archetype(int id)
{
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (item.script_object.Get().Get()->Get_Symbol()->symbol->unique_id == id)
		{
			return &item;
		}
	}
	return NULL;
}

ItemArchetype* ItemManager::Find_Archetype(CVMLinkedSymbol* symbol)
{
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (item.script_object.Get().Get()->Get_Symbol() == symbol)
		{
			return &item;
		}
	}
	return NULL;
}

std::vector<ItemArchetype*> ItemManager::Get_Archetypes()
{
	std::vector<ItemArchetype*> result;
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		result.push_back(&item);
	}
	return result;
}

ItemUpgradeTree* ItemManager::Find_Upgrade_Tree(CVMLinkedSymbol* symbol)
{
	for (std::vector<ItemUpgradeTree>::iterator iter = m_upgrade_trees.begin(); iter != m_upgrade_trees.end(); iter++)
	{
		ItemUpgradeTree& item = *iter;
		if (item.script_object.Get().Get()->Get_Symbol() == symbol)
		{
			return &item;
		}
	}
	return NULL;
}

struct ItemDropBundle
{
	std::vector<ItemArchetype*> types;
	int rarity;
	int bundle_id;
	int probability;
};

void ItemManager::Dump_Item_Schema_Generator(std::string& schema, std::string type, int base_id)
{
	// Keep in sync with Item_Rarity
	const char* rarity_names[] = {
		"COMMON",
		"UNCOMMON",
		"RARE",
		"ULTRA_RARE",
		"LEGENDARY",
	};

	std::vector<ItemDropBundle> bundles;

	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;
		if (S(item.category) == type && item.is_inventory_droppable)
		{
			bool bNewRarity = true;

			for (std::vector<ItemDropBundle>::iterator iter2 = bundles.begin(); iter2 != bundles.end(); iter2++)
			{
				ItemDropBundle& bundle = *iter2;
				if (bundle.rarity == item.rarity)
				{
					bNewRarity = false;
					bundle.types.push_back(&item);
					break;
				}
			}

			if (bNewRarity)
			{
				ItemDropBundle bundle;
				bundle.types.push_back(&item);
				bundle.rarity = item.rarity;
				bundle.bundle_id = base_id + (bundles.size() + 1);

				switch (item.rarity)
				{
					case ItemRarity::Legendary:
					{
						bundle.probability = 5;
						break;
					}
					case ItemRarity::Ultra_Rare:
					{
						bundle.probability = 10;
						break;
					}
					case ItemRarity::Rare:
					{
						bundle.probability = 20;
						break;
					}
					case ItemRarity::Uncommon:
					{
						bundle.probability = 50;
						break;
					}
					case ItemRarity::Common:
						// fallthrough
					default:
					{
						bundle.probability = 100;
						break;
					}
				}
				bundles.push_back(bundle);
			}
		}
	}

	std::string base_bundle = "";
	int base_total = 0;

	for (std::vector<ItemDropBundle>::iterator iter2 = bundles.begin(); iter2 != bundles.end(); iter2++)
	{
		ItemDropBundle& bundle = *iter2;
		base_total += bundle.probability;
	}

	float single_percent = 100.0f / base_total;

	for (std::vector<ItemDropBundle>::iterator iter2 = bundles.begin(); iter2 != bundles.end(); iter2++)
	{
		ItemDropBundle& bundle = *iter2;
		if (base_bundle != "")
		{
			base_bundle += ";";
		}

		int drop_rate = (int)(bundle.probability * single_percent);
		base_bundle += StringHelper::Format("%ix%i", bundle.bundle_id, drop_rate);
	}

	// Base generator.
	schema += "\t{\n";
	schema += StringHelper::Format("\t\t\"name\":				\"%s_GENERATOR\",\n", StringHelper::Uppercase(type.c_str()).c_str());
	schema += StringHelper::Format("\t\t\"itemdefid\":			%i,\n", base_id);
	schema += StringHelper::Format("\t\t\"type\":				 \"generator\",\n");
	schema += StringHelper::Format("\t\t\"name\":				\"%s Generator\",\n", type.c_str());
	schema += StringHelper::Format("\t\t\"market_name\":		\"%s Generator\",\n", type.c_str());
	schema += StringHelper::Format("\t\t\"market_hash_name\":	\"%s Generator\",\n", type.c_str());
	schema += StringHelper::Format("\t\t\"bundle\":				\"%s\",\n", base_bundle.c_str());
	schema += StringHelper::Format("\t\t\"price\":				\"1;VLV10000\",\n");
	schema += StringHelper::Format("\t\t\"description\":		 \"Generates items based on playtime.\",\n");
	schema += StringHelper::Format("\t\t\"name_color\":			\"AF60FF\",\n");
	schema += StringHelper::Format("\t\t\"background_color\":	\"3C352E\",\n");
	schema += StringHelper::Format("\t\t\"item_name\":			\"%s Generator\",\n", type.c_str());
	schema += StringHelper::Format("\t\t\"proper_name\":		false,\n");
	schema += StringHelper::Format("\t\t\"item_slot\":			\"none\",\n");
	schema += StringHelper::Format("\t\t\"item_quality\":		0,\n");
	schema += StringHelper::Format("\t\t\"icon_url\":			\"http://www.zombiegrinder.com/items/icons/small/unknown.png\",\n");
	schema += StringHelper::Format("\t\t\"icon_url_large\":		\"http://www.zombiegrinder.com/items/icons/large/unknown.png\",\n");
	schema += StringHelper::Format("\t\t\"marketable\":			false,\n");
	schema += StringHelper::Format("\t\t\"tradable\":			false,\n");
	schema += StringHelper::Format("\t\t\"commodity\":			false\n");
	schema += "\t},\n";

	// Rarity level generators.
	for (std::vector<ItemDropBundle>::iterator iter2 = bundles.begin(); iter2 != bundles.end(); iter2++)
	{
		ItemDropBundle& bundle = *iter2;

		float single_bundle_percent = 100.0f / bundle.types.size();

		std::string rarity_bundle = "";

		for (std::vector<ItemArchetype*>::iterator iter3 = bundle.types.begin(); iter3 != bundle.types.end(); iter3++)
		{
			ItemArchetype* type2 = *iter3;
			if (rarity_bundle != "")
			{
				rarity_bundle += ";";
			}

			rarity_bundle += StringHelper::Format("%ix%i", type2->Get_Inventory_ItemDefID(), 1);//(int)single_bundle_percent);
		}

		schema += "\t{\n";
		schema += StringHelper::Format("\t\t\"name\":				\"%s_RARITY_%s_GENERATOR\",\n", StringHelper::Uppercase(type.c_str()).c_str(), rarity_names[bundle.rarity]);
		schema += StringHelper::Format("\t\t\"itemdefid\":			%i,\n", bundle.bundle_id);
		schema += StringHelper::Format("\t\t\"type\":				 \"generator\",\n");
		schema += StringHelper::Format("\t\t\"name\":				\"%s %s Generator\",\n", type.c_str(), rarity_names[bundle.rarity]);
		schema += StringHelper::Format("\t\t\"market_name\":		\"%s %s Generator\",\n", type.c_str(), rarity_names[bundle.rarity]);
		schema += StringHelper::Format("\t\t\"market_hash_name\":	\"%s %s Generator\",\n", type.c_str(), rarity_names[bundle.rarity]);
		schema += StringHelper::Format("\t\t\"bundle\":				\"%s\",\n", rarity_bundle.c_str());
		schema += StringHelper::Format("\t\t\"price\":				\"1;VLV10000\",\n");
		schema += StringHelper::Format("\t\t\"description\":		 \"Generates items based on playtime.\",\n");
		schema += StringHelper::Format("\t\t\"name_color\":			\"AF60FF\",\n");
		schema += StringHelper::Format("\t\t\"background_color\":	\"3C352E\",\n");
		schema += StringHelper::Format("\t\t\"item_name\":			\"%s %s Generator\",\n", type.c_str(), rarity_names[bundle.rarity]);
		schema += StringHelper::Format("\t\t\"proper_name\":		false,\n");
		schema += StringHelper::Format("\t\t\"item_slot\":			\"none\",\n");
		schema += StringHelper::Format("\t\t\"item_quality\":		0,\n");
		schema += StringHelper::Format("\t\t\"icon_url\":			\"http://www.zombiegrinder.com/items/icons/small/unknown.png\",\n");
		schema += StringHelper::Format("\t\t\"icon_url_large\":		\"http://www.zombiegrinder.com/items/icons/large/unknown.png\",\n");
		schema += StringHelper::Format("\t\t\"marketable\":			false,\n");
		schema += StringHelper::Format("\t\t\"tradable\":			false,\n");
		schema += StringHelper::Format("\t\t\"commodity\":			false\n");
		schema += "\t},\n";
	}
}

void ItemManager::Dump_Item_Schema()
{
	std::string output_dir = Platform::Get()->Get_Absolute_Path("/items/");
	std::string large_icon_dir = Platform::Get()->Get_Absolute_Path("/items/icons/large/");
	std::string small_icon_dir = Platform::Get()->Get_Absolute_Path("/items/icons/small/");
	Platform::Get()->Create_Directory(output_dir.c_str(), true);
	Platform::Get()->Create_Directory(large_icon_dir.c_str(), true);
	Platform::Get()->Create_Directory(small_icon_dir.c_str(), true);

	DBG_LOG("Dumping item schema ...");

	const int small_icon_size = 96;
	const int large_icon_size = 200;

	std::string schema = "";

	std::string accessories_bundle = "";
	std::string gems_bundle = "";
	std::string hats_bundle = "";
	std::string paints_bundle = "";
	std::string weapons_bundle = "";

	// TODO:
	// Blah this is a shitty way of generating json. Use a proper
	// constructor plz.
	schema += "{\n";
	schema += "\t\"appid\": 263920,\n";
	schema += "\t\"items\": [\n";

	// Item drop generator.
	schema += "\t{\n";
	schema += StringHelper::Format("\t\t\"name\":				\"DROP_GENERATOR\",\n");
	schema += StringHelper::Format("\t\t\"itemdefid\":			%i,\n", INVENTORY_ITEM_PLAYTIME_DROP_ID);
	schema += StringHelper::Format("\t\t\"type\":				 \"playtimegenerator\",\n");
	schema += StringHelper::Format("\t\t\"name\":				\"Drop Generator\",\n");
	schema += StringHelper::Format("\t\t\"market_name\":		\"Drop Generator\",\n");
	schema += StringHelper::Format("\t\t\"market_hash_name\":	\"Drop Generator\",\n");
	schema += StringHelper::Format("\t\t\"bundle\":				\"%ix%i;%ix%i;%ix%i;%ix%i;%ix%i\",\n",
		INVENTORY_ITEM_ACCESSORIES_DROP_ID, INVENTORY_ITEM_ACCESSORIES_PROBABILITY,
		INVENTORY_ITEM_GEMS_DROP_ID, INVENTORY_ITEM_GEMS_PROBABILITY,
		INVENTORY_ITEM_HATS_DROP_ID, INVENTORY_ITEM_HATS_PROBABILITY,
		INVENTORY_ITEM_PAINTS_DROP_ID, INVENTORY_ITEM_PAINTS_PROBABILITY,
		INVENTORY_ITEM_WEAPONS_DROP_ID, INVENTORY_ITEM_WEAPONS_PROBABILITY);
	schema += StringHelper::Format("\t\t\"price\":				\"1;VLV10000\",\n");
	schema += StringHelper::Format("\t\t\"description\":		 \"Generates items based on playtime.\",\n");
	schema += StringHelper::Format("\t\t\"name_color\":			\"AF60FF\",\n");
	schema += StringHelper::Format("\t\t\"background_color\":	\"3C352E\",\n");
	schema += StringHelper::Format("\t\t\"item_name\":			\"Drop Generator\",\n");
	schema += StringHelper::Format("\t\t\"proper_name\":		false,\n");
	schema += StringHelper::Format("\t\t\"item_slot\":			\"none\",\n");
	schema += StringHelper::Format("\t\t\"item_quality\":		0,\n");
	schema += StringHelper::Format("\t\t\"icon_url\":			\"http://www.zombiegrinder.com/items/icons/small/unknown.png\",\n");
	schema += StringHelper::Format("\t\t\"icon_url_large\":		\"http://www.zombiegrinder.com/items/icons/large/unknown.png\",\n");
	schema += StringHelper::Format("\t\t\"marketable\":			false,\n");
	schema += StringHelper::Format("\t\t\"tradable\":			false,\n");
	schema += StringHelper::Format("\t\t\"commodity\":			false\n");
	schema += "\t},\n";

	// Item bundles.
	Dump_Item_Schema_Generator(schema, "Accessories", INVENTORY_ITEM_ACCESSORIES_DROP_ID);
	Dump_Item_Schema_Generator(schema, "Gems", INVENTORY_ITEM_GEMS_DROP_ID);
	Dump_Item_Schema_Generator(schema, "Hats", INVENTORY_ITEM_HATS_DROP_ID);
	Dump_Item_Schema_Generator(schema, "Paints", INVENTORY_ITEM_PAINTS_DROP_ID);
	Dump_Item_Schema_Generator(schema, "Items", INVENTORY_ITEM_WEAPONS_DROP_ID);


	for (int i = 0; i < 99; i++)
	{
		DBG_LOG("==== Rank %i ====", i);
		for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
		{
			ItemArchetype& item = *iter;
			if (item.unlock_critera == ItemUnlockCriteria::Required_Rank)
			{
				if (item.unlock_critera_threshold == i)
				{
					DBG_LOG("Item: %s", item.base_name.c_str());
				}
			}
		}
	}

	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& item = *iter;

		Pixelmap* small_icon = new Pixelmap(small_icon_size, small_icon_size, PixelmapFormat::R8G8B8A8);
		Pixelmap* scaled_small_icon = new Pixelmap(small_icon_size, small_icon_size, PixelmapFormat::R8G8B8A8);
		Pixelmap* large_icon = new Pixelmap(large_icon_size, large_icon_size, PixelmapFormat::R8G8B8A8);

		small_icon->Clear(Color(255, 255, 255, 0));
		scaled_small_icon->Clear(Color(255, 255, 255, 0));
		large_icon->Clear(Color(255, 255, 255, 0));

		int direction = 0;

		if (item.is_icon_direction_based == true)
		{
			direction = Directions::S;
		}
		else
		{
			direction = 0;
		}

		// Generate small icon.
		AtlasFrame* frame = item.icon_animations[0]->Frames[0];
		Vector2 frame_position = Vector2((small_icon_size) / 2 , (small_icon_size) / 2) - Vector2(item.icon_offset.X, item.icon_offset.Y);
		small_icon->Paste(frame_position, frame->TexturePtr->PixelmapPtr->Window(frame->Rect));
		if (item.is_tintable)
		{
			frame = item.icon_tint_animations[0]->Frames[0];
			small_icon->PasteColorBlended(item.default_tint, frame_position, frame->TexturePtr->PixelmapPtr->Window(frame->Rect));
		}

		// Generate scaled-small icon.
		frame = item.icon_animations[0]->Frames[0];
		int scaled_small_icon_size = small_icon_size * 2;
		frame_position = Vector2((small_icon_size - scaled_small_icon_size) / 2 , (small_icon_size - scaled_small_icon_size) / 2);
		scaled_small_icon->PasteScaled(Vector2(2.0f, 2.0f), frame_position, small_icon->Window(Rect2D(0, 0, small_icon->Get_Width(), small_icon->Get_Height())));

		// Generate large icon.
		frame = item.icon_animations[0]->Frames[0];
		scaled_small_icon_size = small_icon_size * 3;
		frame_position = Vector2((large_icon_size - scaled_small_icon_size) / 2 , (large_icon_size - scaled_small_icon_size) / 2);
		large_icon->PasteScaled(Vector2(3.0f, 3.0f), frame_position, small_icon->Window(Rect2D(0, 0, small_icon->Get_Width(), small_icon->Get_Height())));

		// Save icons.
		std::string icon_filename = StringHelper::Format("%08x.png", item.script_object.Get().Get()->Get_Symbol()->name_hash);
		PixelmapFactory::Save((small_icon_dir + icon_filename).c_str(), scaled_small_icon);
		PixelmapFactory::Save((large_icon_dir + icon_filename).c_str(), large_icon);

		// Add item to schema.
		schema += "\t{\n";

		/*std::string name = SF("#item_indestructable_prefix", S(item.base_name.c_str()));
		if (item.item_slot == (int)ItemSlot::All)
		{
			name = S(item.base_name.c_str());
		}		
		if (item.combine_group == "Paints")
		{
			name = SF("#item_unlimited_prefix", S(item.base_name.c_str()));
		}
		*/

		std::string name = item.Get_Display_Name(true, 0, true);
		name = StringHelper::Replace(name.c_str(), "\"", "");

		std::string description = item.Get_Description(true);

		ItemUpgradeTree* tree = item.Get_Upgrade_Tree();
		if (tree)
		{
			std::vector<ItemUpgradeTreeNodeModifier> modifiers;
			std::vector<int> upgrade_ids;
			std::vector<ItemUpgradeTreeNode*> nodes;

			tree->Get_Nodes(nodes);

			for (std::vector<ItemUpgradeTreeNode*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
			{
				ItemUpgradeTreeNode* node = *iter;
				upgrade_ids.push_back(node->id);
			}

			tree->Get_Active_Compound_Modifiers(modifiers, upgrade_ids);

			description += "\n\n";

			for (std::vector<ItemUpgradeTreeNodeModifier>::iterator iter = modifiers.begin(); iter != modifiers.end(); iter++)
			{
				ItemUpgradeTreeNodeModifier& mod = *iter;
				description += mod.To_BBCode_Description_String() + "\n";
			}

			description += S("#weapon_upgrade_mod_infinite_durability_description_bbcode");
			description += "\n";
		}
		else if (item.item_slot != ItemSlot::All)
		{
			description += "\n\n";
			description += S("#weapon_upgrade_mod_infinite_durability_description_bbcode");
			description += "\n";
		}


		DBG_LOG("Dumped: %s: %s", name.c_str(), description.c_str());

		description = StringHelper::Replace(description.c_str(), "\"", "");
		description = StringHelper::Replace(description.c_str(), "\r", "");
		description = StringHelper::Replace(description.c_str(), "\n", "\\n");

		schema += StringHelper::Format("\t\t\"name\": \"%s\",\n",															item.script_object.Get().Get()->Get_Symbol()->symbol->name);
		schema += StringHelper::Format("\t\t\"itemdefid\": %i,\n",															item.Get_Inventory_ItemDefID());
		schema += StringHelper::Format("\t\t\"type\": \"%s\",\n",															item.combine_group.c_str());
		schema += StringHelper::Format("\t\t\"name\": \"%s\",\n",															name.c_str());
		schema += StringHelper::Format("\t\t\"market_name\": \"%s\",\n",													name.c_str());
		schema += StringHelper::Format("\t\t\"market_hash_name\": \"%s\",\n",												name.c_str());
		schema += StringHelper::Format("\t\t\"price\": \"%s\",\n",															item.premium_price.c_str());
		schema += StringHelper::Format("\t\t\"description\": \"%s\",\n",													description.c_str());

		switch (item.rarity)
		{
		case ItemRarity::Common:		
			{
				schema += StringHelper::Format("\t\t\"name_color\": \"%s\",\n",												"4FC678");
				schema += StringHelper::Format("\t\t\"background_color\": \"%s\",\n",										"3C352E");
				break;
			}
		case ItemRarity::Uncommon:		
			{
				schema += StringHelper::Format("\t\t\"name_color\": \"%s\",\n",												"8888FF");
				schema += StringHelper::Format("\t\t\"background_color\": \"%s\",\n",										"3C352E");
				break;
			}
		case ItemRarity::Rare:		
			{
				schema += StringHelper::Format("\t\t\"name_color\": \"%s\",\n",												"AE1C1C");
				schema += StringHelper::Format("\t\t\"background_color\": \"%s\",\n",										"3C352E");
				break;
			}
		case ItemRarity::Ultra_Rare:		
			{
				schema += StringHelper::Format("\t\t\"name_color\": \"%s\",\n",												"FFFF17");
				schema += StringHelper::Format("\t\t\"background_color\": \"%s\",\n",										"3C352E");
				break;
			}
		case ItemRarity::Legendary:		
			{
				schema += StringHelper::Format("\t\t\"name_color\": \"%s\",\n",												"AF60FF");
				schema += StringHelper::Format("\t\t\"background_color\": \"%s\",\n",										"3C352E");
				break;
			}
		}

		schema += StringHelper::Format("\t\t\"item_name\": \"%s\",\n",														item.script_object.Get().Get()->Get_Symbol()->symbol->name);

		schema += StringHelper::Format("\t\t\"proper_name\": %s,\n",														"false");

#ifdef OPT_PREMIUM_ACCOUNTS
		if (item.is_premium_only)
		{
			schema += StringHelper::Format("\t\t\"promo\": \"owns:%i\",\n", 321500);
		}
#else
		schema += StringHelper::Format("\t\t\"promo\": \"\",\n");
#endif

		schema += StringHelper::Format("\t\t\"item_slot\": \"%s\",\n",															"decoration");
		schema += StringHelper::Format("\t\t\"item_quality\": %i,\n",														item.rarity);

		schema += StringHelper::Format("\t\t\"icon_url\": \"http://www.zombiegrinder.com/items/icons/small/%s\",\n",		icon_filename.c_str());
		schema += StringHelper::Format("\t\t\"icon_url_large\": \"http://www.zombiegrinder.com/items/icons/large/%s\",\n",	icon_filename.c_str());

		schema += StringHelper::Format("\t\t\"marketable\": %s,\n",		item.is_tradable ? "true" : "false");
		schema += StringHelper::Format("\t\t\"tradable\": %s,\n",		item.is_tradable ? "true" : "false");
		schema += StringHelper::Format("\t\t\"commodity\": %s\n",		"true");

		if (iter != m_archetypes.end() - 1)
		{
			schema += "\t},\n";
		}
		else
		{
			schema += "\t}\n";
		}

		// Clean up.
		SAFE_DELETE(small_icon);
		SAFE_DELETE(scaled_small_icon);
		SAFE_DELETE(large_icon);
	}

	schema += "\t]\n";
	schema += "}\n";

	DBG_LOG("Dump complete.");

	StreamFactory::Save_Text((output_dir + "schema.xml").c_str(), schema);
}

bool ItemManager::Init()
{	
	DBG_ASSERT(!m_init);
	DBG_LOG("Loading items ...");

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	m_item_class = vm->Find_Class("Item");
	m_item_archetype_class = vm->Find_Class("Item_Archetype");
	m_item_upgrade_tree_class = vm->Find_Class("Item_Upgrade_Tree");
	DBG_ASSERT(m_item_class != NULL);
	DBG_ASSERT(m_item_archetype_class != NULL);
	DBG_ASSERT(m_item_upgrade_tree_class != NULL);
	
	std::vector<CVMLinkedSymbol*> archetype_extended_classes = vm->Find_Derived_Classes(m_item_archetype_class);
	m_archetypes.resize(archetype_extended_classes.size());

	std::vector<CVMLinkedSymbol*> upgrade_tree_extended_classes = vm->Find_Derived_Classes(m_item_upgrade_tree_class);
	m_upgrade_trees.resize(upgrade_tree_extended_classes.size());

	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	bool success = true;
	int index = 0;

	for (std::vector<CVMLinkedSymbol*>::iterator iter = upgrade_tree_extended_classes.begin(); iter != upgrade_tree_extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* item_class = *iter;
		DBG_LOG("Loading Upgrade Tree Class: %s", item_class->symbol->name);

		ItemUpgradeTree& item = m_upgrade_trees.at(index);
		vm->Get_Static_Context()->MetaData = &item;

		item.script_object = vm->New_Object(item_class, true, &item);
		vm->Set_Default_State(item.script_object);
	}

	index = 0;

	for (std::vector<CVMLinkedSymbol*>::iterator iter = archetype_extended_classes.begin(); iter != archetype_extended_classes.end(); iter++, index++)
	{
		CVMLinkedSymbol* item_class = *iter;
		DBG_LOG("Loading Item Class: %s", item_class->symbol->name);

		ItemArchetype& item = m_archetypes.at(index);
		item.combination_count = 0;
		item.ammo_type = NULL;
		item.weapon_type = NULL;
		item.post_process_fx_type = NULL;
		item.is_unpackable = false;

		vm->Get_Static_Context()->MetaData = &item;

		item.script_object = vm->New_Object(item_class, true, &item);
		vm->Set_Default_State(item.script_object);
		
#define LOAD_ANIMATION(dest, name) \
		dest = ResourceFactory::Get()->Get_Atlas_Animation((name).c_str()); \
		if (dest == NULL) \
		{ \
			DBG_LOG("[Warning] Could not find item frame '%s'.", (name).c_str()); \
			success = false;\
		} 				

		// Load atlas frames.
		if (item.is_icon_direction_based == true)
		{
			for (int i = 0; i < 8; i++)
			{
				LOAD_ANIMATION(item.icon_animations[i], item.icon_animation_name + "_" + Direction_Short_Mnemonics[i])
				if (item.is_tintable)
				{
					LOAD_ANIMATION(item.icon_tint_animations[i], item.icon_animation_name + "_tint_" + Direction_Short_Mnemonics[i])
				}
			}
		}
		else
		{
			LOAD_ANIMATION(item.icon_animations[0], item.icon_animation_name)
			if (item.is_tintable)
			{
				LOAD_ANIMATION(item.icon_tint_animations[0], item.icon_animation_name + "_tint")
			}
		}

		// Load body.
		if (item.body_animation_name != "")
		{
			for (int i = 0; i < 8; i++)
			{
				LOAD_ANIMATION(item.body_animations[i], item.body_animation_name + "_" + Direction_Short_Mnemonics[i])
				if (item.is_tintable)
				{
					LOAD_ANIMATION(item.body_tint_animations[i], item.body_animation_name + "_tint_" + Direction_Short_Mnemonics[i])
				}
			}
		}

#undef LOAD_ANIMATION
	}
	
	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& type = *iter;
		type.is_combinable = false;
	}

	for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
	{
		ItemArchetype& type = *iter;
		type.is_equippable = (type.item_slot >= 0 && type.item_slot < ItemSlot::All);

		if (type.is_combinable == false)
		{
			type.is_combinable = (type.combination_count > 0);
		}
		
		for (int i = 0; i < type.combination_count; i++)
		{
			ItemCombination& combo = type.combinations[i];

			for (std::vector<ItemArchetype>::iterator iter2 = m_archetypes.begin(); iter2 != m_archetypes.end(); iter2++)
			{
				ItemArchetype& item = *iter2;
				if (strcmp(item.combine_group.c_str(), combo.other_group.c_str()) == 0 ||
					strcmp(item.combine_name.c_str(), combo.other_group.c_str()) == 0)
				{
					item.is_combinable = true;
				}
			}
		}
	}

	if (*EngineOptions::generate_item_schema)
	{
		Dump_Item_Schema();
	}

	if (*EngineOptions::give_self_all_items)
	{
		if (OnlineInventory::Try_Get() != NULL)
		{
			for (std::vector<ItemArchetype>::iterator iter = m_archetypes.begin(); iter != m_archetypes.end(); iter++)
			{
				ItemArchetype& type = *iter;
				OnlineInventory::Get()->GiveSelf(type.Get_Inventory_ItemDefID());
			}
		}
	}

	DBG_ASSERT(success);

	m_init = true;
	return true;
}
