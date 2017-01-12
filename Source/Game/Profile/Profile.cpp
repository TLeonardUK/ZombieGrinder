// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#include "Game/Profile/Profile.h"
#include "Game/Profile/ProfileManager.h"
#include "Game/Profile/ItemManager.h"
#include "Game/Profile/SkillManager.h"
#include "Game/Online/DLCManager.h"

#include "Engine/Online/OnlineInventory.h"

#include "Game/Runner/Game.h"

#include "Engine/Localise/Locale.h"

#include "Engine/Engine/GameEngine.h"
#include "Engine/Scripts/EngineVirtualMachine.h"

#include "XScript/VirtualMachine/CVMValue.h"
#include "XScript/VirtualMachine/CVMContext.h"
#include "XScript/VirtualMachine/CVMBinary.h"

#include "Game/Scripts/ScriptEventListener.h"

// These are ranges used by randomised item stats, be careful with these, they require a lot of balancing!
ProfileStatRange Profile_Stat::Ranges[Profile_Stat::COUNT] = {
	ProfileStatRange(0.0f, 0.0, 1.0f, -1.0f, 0.0f), //Wallet_Interest,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.2f, 1.0f), //Max_Health,
	ProfileStatRange(0.0f, 0.0, 0.3f, -0.3f, 0.0f), //Health_Regen,
	ProfileStatRange(1.0f, 1.0, 1.25f, 0.1f, 1.0f), //Speed,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.1f, 1.0f), //Ammo_Capacity,
	ProfileStatRange(0.0f, 0.0, 0.3f, -0.3f, 0.0f), //Ammo_Regen,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.1f, 1.0f), //Gold_Drop,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.1f, 1.0f), //Gold_Magnet,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.1f, 1.0f), //Buy_Price,
	ProfileStatRange(0.5f, 0.5, 1.5f, 0.1f, 0.5f), //Sell_Price,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.5f, 1.0f), //Gem_Drop,
	ProfileStatRange(1.0f, 1.0, 1.5f, 0.5f, 1.0f), //Gem_Magnet,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //XP,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Melee_Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Melee_Stamina,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Rifle_Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Rifle_Ammo,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Heavy_Weapon_Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Heavy_Weapon_Ammo,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Explosion_Radius,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Enemy_Status_Duration,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Turret_Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Turret_Ammo,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Small_Arms_Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Small_Arms_Ammo,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Buff_Grenade_Radius,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Buff_Grenade_Ammo,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Ghost_Life_Time,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Ghost_Revive_Time,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Healing_Speed,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Damage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Damage_Taken,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Rate_Of_Fire,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Reload_Speed,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Ammo_Usage,
	ProfileStatRange(1.0f, 1.0, 2.0f, 0.5f, 1.0f), //Targetable,
};

const char* Profile_Stat::Names[Profile_Stat::COUNT] = {
	"#profile_stat_name_wallet_interest",
	"#profile_stat_name_max_health",
	"#profile_stat_name_health_regen",
	"#profile_stat_name_speed",
	"#profile_stat_name_ammo_capacity",
	"#profile_stat_name_ammo_regen",
	"#profile_stat_name_gold_drop",
	"#profile_stat_name_gold_magnet",
	"#profile_stat_name_buy_price",
	"#profile_stat_name_sell_price",
	"#profile_stat_name_gem_drop",
	"#profile_stat_name_gem_magnet",
	"#profile_stat_name_xp",
	"#profile_stat_name_melee_damage",
	"#profile_stat_name_melee_stamina",
	"#profile_stat_name_rifle_damage",
	"#profile_stat_name_rifle_ammo",
	"#profile_stat_name_heavy_weapon_damage",
	"#profile_stat_name_heavy_weapon_ammo",
	"#profile_stat_name_explosion_radius",
	"#profile_stat_name_enemy_status_duration",
	"#profile_stat_name_turret_damage",
	"#profile_stat_name_turret_ammo",
	"#profile_stat_name_small_arms_damage",
	"#profile_stat_name_small_arms_ammo",
	"#profile_stat_name_buff_grenade_radius",
	"#profile_stat_name_buff_grenade_ammo",
	"#profile_stat_name_ghost_life_time",
	"#profile_stat_name_ghost_revive_time",
	"#profile_stat_name_healing_speed",
	"#profile_stat_name_damage",
	"#profile_stat_name_damage_taken",
	"#profile_stat_name_rate_of_fire",
	"#profile_stat_name_reload_speed",
	"#profile_stat_name_ammo_usage",
	"#profile_stat_name_targetable"
};

bool ProfileItemStack::Sort_Predicate(const ProfileItemStack& a, const ProfileItemStack& b)
{
	if (a.Archetype->item_slot == b.Archetype->item_slot)
	{
		int cat_a = a.category_id;
		int cat_b = b.category_id;
		if (cat_a == cat_b)
		{
			return (a.name_id < b.name_id);
		}
		else
		{
			return (cat_a < cat_b);
		}
	}
	else
	{
		return (a.Archetype->item_slot < b.Archetype->item_slot);
	}
}

Profile::Profile()
	: Level(1)
	, Level_XP(0)
	, Name("Hopefully Unreadable Text")
	, Coins_Interest(0.0f)
	, Unique_ID_Counter(1)
	, Is_Male(true)
	, Inventory_Space(0)
	, Inventory_Space_Dirty(true)
	, Item_Slots_Dirty(true)
{
	Profile_Item_Stat_Class = GameEngine::Get()->Get_VM()->Find_Class("Profile_Item_Stat");
	DBG_ASSERT(Profile_Item_Stat_Class != NULL);

	Profile_Skill_Energy_Class = GameEngine::Get()->Get_VM()->Find_Class("Profile_Skill_Energy");
	DBG_ASSERT(Profile_Skill_Energy_Class != NULL);
}

void Profile::Sanitize()
{
	std::vector<ItemArchetype*> archetypes = ItemManager::Get()->Get_Archetypes();

	// First thing we do is update the archetype of any inventory drop items who
	// have been retired/replaced.
	CVMObject* item_arr = Item_Array.Get().Get();
	for (int i = 0; i < item_arr->Slot_Count(); i++)
	{
		CVMObject* obj = item_arr->Get_Slot(i).object_value.Get();
		if (obj != NULL)
		{
			Item* item = reinterpret_cast<Item*>(obj->Get_Meta_Data());

			if (item->was_inventory_drop)
			{
				if (item->archetype->Get_Inventory_ItemDefID() != item->archetype->Get_Not_Overriden_Inventory_ItemDefID())
				{
					ItemArchetype* new_archetype = ItemManager::Get()->Find_Archetype_By_InventoryId(item->archetype->Get_Not_Overriden_Inventory_ItemDefID());
					if (new_archetype != item->archetype && new_archetype != NULL)
					{
						DBG_LOG("Item %i in profile has had inventory itemdefid upgraded. Replaced archetype of '%s' with '%s'.", 
							i, 
							item->archetype == NULL ? "NULL" : S(item->archetype->base_name),
							new_archetype == NULL ? "NULL" : S(new_archetype->base_name));

						item->archetype = new_archetype;
					}
				}
			}
		}
	}

	// Find cheapest most generic items to use as defaults.
	ItemArchetype* default_archetypes[64];
	for (int i = 0; i < 64; i++)
	{
		default_archetypes[i] = NULL;

		for (std::vector<ItemArchetype*>::iterator iter = archetypes.begin(); iter != archetypes.end(); iter++)
		{
			ItemArchetype* arch = *iter;
			if (arch->item_slot == i && arch->is_buyable)
			{
				if (default_archetypes[i] == NULL || arch->cost < default_archetypes[i]->cost)
				{
					default_archetypes[i] = arch;
				}
			}
		}
	}

	// Remove invalid items that may be due to remove items, or mods.
	for (int i = 0; i < item_arr->Slot_Count(); i++)
	{
		CVMObject* obj = item_arr->Get_Slot(i).object_value.Get();
		if (obj != NULL)
		{
			Item* item = reinterpret_cast<Item*>(obj->Get_Meta_Data());

			if (item->archetype == NULL)
			{
				if (item->equip_slot >= 0)
				{
					item->archetype = default_archetypes[item->equip_slot];
				}
				else
				{
					item->archetype = default_archetypes[0];
				}

				DBG_LOG("Item %i in profile has no archetype, possible orphan from removed mod? Replaced with a '%s'.", i, S(item->archetype->base_name));
			}
		}
	}

	// Seperate items that are combined in invalid combinations.
	for (int i = 0; i < item_arr->Slot_Count(); i++)
	{
		CVMObject* obj = item_arr->Get_Slot(i).object_value.Get();
		if (obj != NULL)
		{
			Item* item = reinterpret_cast<Item*>(obj->Get_Meta_Data());
			std::vector<Item*> attached = Get_Attached_Items(item);

			// Remove any invalid combinations.
			for (std::vector<Item*>::iterator iter = attached.begin(); iter != attached.end(); iter++)
			{
				Item* other_item = *iter;
				if (!Can_Combine_Item(item, other_item))
				{
					other_item->attached_to_id = -1;
					Inventory_Space_Dirty = true;
				}
			}
		}
	}

	// Call script sanitizations.
	{
		EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
		vm->Get_Static_Context()->MetaData = this;
		CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

		// Call rollback method to reset things.
		CVMLinkedSymbol* archetype_class = vm->Find_Class("Profile");
		CVMLinkedSymbol* event = vm->Find_Function(archetype_class, "Sanitize", 0);
		CVMValue instance_value = Script_Object.Get();
		vm->Invoke(event, instance_value, false, false, false);
	}

	// Update any skills that have changed since previous versions.

	{
		EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
		vm->Get_Static_Context()->MetaData = this;
		CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

		CVMLinkedSymbol* archetype_class = vm->Find_Class("Skill_Archetype");
		CVMLinkedSymbol* onrecieve_event = vm->Find_Function(archetype_class, "On_Loaded", 2, "Profile", "Skill");

		CVMObject* item_array = Skill_Array.Get().Get();
		for (int i = 0; i < item_array->Slot_Count(); i++)
		{
			Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());

			// Invoke the On_Load event.
			CVMValue param_1 = Script_Object.Get();
			CVMValue param_2 = item_array->Get_Slot(i);
			CVMValue instance = item->archetype->script_object.Get();
			vm->Push_Parameter(param_1);
			vm->Push_Parameter(param_2);
			vm->Invoke(onrecieve_event, instance, false, false, false);
		}
	}

	// Sanitise some inventory items post-updates.
	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		sub_item->indestructable = sub_item->indestructable || sub_item->was_inventory_drop;
	}

	// Ensure XP is within bounds.
	Level_XP = Max(0, Level_XP);

	// Make sure we have a head equipped!
	Ensure_Valid_Equips();
}

float Profile::Get_Level_Progress()
{
	int xp_required = ProfileManager::Get()->Get_Level_XP(Level + 1);
	return (float)Level_XP / (float)xp_required;
}

int Profile::Get_Total_Items()
{
	return Item_Array.Get().Get()->Slot_Count();
}

int Profile::Get_Inventory_Space()
{
	if (Inventory_Space_Dirty)
	{
		Inventory_Space = Get_Inventory_Size() - Get_Item_Stacks().size();
		Inventory_Space_Dirty = false;
	}
	return Inventory_Space;
}

int Profile::Get_Inventory_Size()
{
	return Inventory_Size;
}

void Profile::Add_Unpacked_Coins(int coins)
{
	ProfileUnpackEvent evt;
	evt.type = ProfileUnpackEventType::Coins;
	evt.amount = coins;
	Unpack_Events.push_back(evt);

	Coins += coins;
}

void Profile::Add_Unpacked_Item(ItemArchetype* type)
{
	ProfileUnpackEvent evt;
	evt.type = ProfileUnpackEventType::Item;
	evt.item = type;
	Unpack_Events.push_back(evt);

	Add_Item(type);
}

void Profile::Add_Unpacked_Fractional_XP(float amount)
{
	ProfileUnpackEvent evt;
	evt.type = ProfileUnpackEventType::XP;
	evt.amount = (int)(ProfileManager::Get()->Get_Level_XP(Level + 1) * amount);
	Unpack_Events.push_back(evt);

	Give_XP(evt.amount);
}

std::vector<Item*> Profile::Get_Items()
{
	std::vector<Item*> retval;

	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		retval.push_back(item);
	}

	return retval;
}

std::vector<ProfileItemStack> Profile::Get_Item_Stacks()
{
	std::vector<ProfileItemStack> stacks;

	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());

		if (item->attached_to_id >= 0)
		{
			continue;
		}

		ProfileItemStack* add_stack = NULL;

		if (item->archetype->is_stackable == true)
		{
			for (std::vector<ProfileItemStack>::iterator iter = stacks.begin(); iter != stacks.end(); iter++)
			{
				ProfileItemStack& stack = *iter;
				if (stack.Archetype == item->archetype &&
					stack.Items[0]->was_inventory_drop == item->was_inventory_drop &&
					(int)stack.Items.size() < item->archetype->max_stack)
				{
					add_stack = &stack;
					break;
				}
			}
		}

		if (add_stack == NULL)
		{
			ProfileItemStack new_stack;
			new_stack.Archetype = item->archetype;
			new_stack.Items.push_back(item);
			new_stack.category_id = StringHelper::Hash(S(item->archetype->category.c_str()));
			new_stack.name_id = StringHelper::Hash(S(item->archetype->base_name.c_str()));
			stacks.push_back(new_stack);
		}
		else
		{
			add_stack->Items.push_back(item);
		}
	}

	// Sort stacks by type.
	std::sort(stacks.begin(), stacks.end(), &ProfileItemStack::Sort_Predicate);

	return stacks;
}

bool Profile::Stack_Space_Available(ItemArchetype* type)
{
	if (!type->is_stackable)
	{
		return false;
	}

	std::vector<ProfileItemStack> stacks = Get_Item_Stacks();
	for (std::vector<ProfileItemStack>::iterator iter = stacks.begin(); iter != stacks.end(); iter++)
	{
		ProfileItemStack& stack = *iter;
		if (stack.Archetype == type && 
			// TODO: Needs to be a check in here for inventory-type.
			(int)stack.Items.size() < type->max_stack)
		{
			return true;
		}
	}

	return false;
}

int Profile::Get_Free_Slots()
{
	return Get_Inventory_Size() - (int)Get_Item_Stacks().size();
}

bool Profile::Is_Space_For_Item(ItemArchetype* type)
{
	std::vector<ProfileItemStack> stacks = Get_Item_Stacks();

	DBG_LOG("Is_Space_For_Item: %i stacks / %i slots.", stacks.size(), Get_Inventory_Size());

	// Is there any free slots?
	if ((int)stacks.size() < Get_Inventory_Size())
	{
		return true;
	}

	// Can we stack the item?
	else if (Stack_Space_Available(type))
	{
		return true;
	}

	// Well your screwed.
	return false;
}

Item* Profile::Add_Item(ItemArchetype* type)
{
	// We should have checked space before adding item. If we haven't
	// then increase the inventory size as its probably a required item (premium upgrade etc).
	if (!Is_Space_For_Item(type))
	{
		Inventory_Size++;
	}

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	vm->Get_Static_Context()->MetaData = this;
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* item_class = vm->Find_Class("Item");
	CVMObjectHandle new_item = vm->New_Object(item_class, true);

	Item* item = reinterpret_cast<Item*>(new_item.Get()->Get_Meta_Data());
	item->archetype			= type;
	item->primary_color		= type->default_tint;
	item->recieve_time		= Platform::Get()->Get_Timestamp();
	item->unique_id			= Get_Unique_ID(); 
	item->attached_to_id	= -1;
	item->script_object		= new_item;

	// Add to inventory array.
	Item_Array.Get().Get()->Add_Last(new_item);
	Inventory_Space_Dirty = true;
	
	// Invoke the On_Recieve event.
	CVMLinkedSymbol* archetype_class = vm->Find_Class("Item_Archetype");
	CVMLinkedSymbol* onrecieve_event = vm->Find_Function(archetype_class, "On_Recieve", 2, "Profile", "Item");
	CVMValue param_1 = Script_Object.Get();
	CVMValue param_2 = new_item;
	CVMValue instance = item->archetype->script_object.Get();
	vm->Push_Parameter(param_1);
	vm->Push_Parameter(param_2);
	vm->Invoke(onrecieve_event, instance, false, false, false);

	DBG_LOG("Added item '%s' to inventory.", S(type->base_name));

	return item;
}

void Profile::Unpack_Item(Item* item)
{
	DBG_LOG("Unpacking item '%s' in inventory.", S(item->archetype->base_name));

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	vm->Get_Static_Context()->MetaData = this;
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	// Invoke the On_Recieve event.
	CVMLinkedSymbol* archetype_class = vm->Find_Class("Item_Archetype");
	CVMLinkedSymbol* onrecieve_event = vm->Find_Function(archetype_class, "On_Unpack", 2, "Profile", "Item");
	CVMValue param_1 = Script_Object.Get();
	CVMValue param_2 = item->script_object;
	CVMValue instance = item->archetype->script_object.Get();
	vm->Push_Parameter(param_1);
	vm->Push_Parameter(param_2);
	vm->Invoke(onrecieve_event, instance, false, false, false);

	// Remove & Consume
	Remove_Item(item, true);
}

void Profile::Ensure_Valid_Equips(Item* ignore_item)
{
	if (Get_Item_Slot(ItemSlot::Head) == NULL)
	{
		bool bFoundSuitableMatch = false;

		CVMObject* item_array = Item_Array.Get().Get();
		for (int i = 0; i < item_array->Slot_Count(); i++)
		{
			Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
			if (ignore_item != sub_item && sub_item->archetype->item_slot == ItemSlot::Head && !sub_item->Is_Broken())
			{
				Equip_Item(sub_item);
				bFoundSuitableMatch = true;
			}
		}
		
		// Ignore ignore item.
		if (!bFoundSuitableMatch)
		{
			for (int i = 0; i < item_array->Slot_Count(); i++)
			{
				Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
				if (sub_item->archetype->item_slot == ItemSlot::Head && !sub_item->Is_Broken())
				{
					Equip_Item(sub_item);
					bFoundSuitableMatch = true;
				}
			}
		}

		// Search for a broken hat we can repair if all else fails.
		if (!bFoundSuitableMatch)
		{
			for (int i = 0; i < item_array->Slot_Count(); i++)
			{
				Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
				if (sub_item->archetype->item_slot == ItemSlot::Head)
				{
					sub_item->equip_duration = 0.0f;
					Equip_Item(sub_item);

					bFoundSuitableMatch = true;
				}
			}
		}

		if (!bFoundSuitableMatch)
		{
			// No hat? Shit, add one and equip it.
			ItemArchetype* base = ItemManager::Get()->Find_Archetype("Item_Player1hair");
			Equip_Item(Add_Item(base));
		}
	}
}

void Profile::Remove_Item(Item* item, bool bConsume)
{
	DBG_LOG("Removed item '%s' to inventory.", S(item->archetype->base_name));

	Unequip_Item(item);

	// Make sure we have a head equipped!
	Ensure_Valid_Equips(item);

	// if its an inventory item we need to erase it.
	if (item->was_inventory_drop && bConsume)
	{
		if (OnlineInventory::Try_Get() != NULL)
		{
			OnlineInventory::Get()->RemoveItem(StringHelper::To_U64(item->inventory_drop_id));
		}
	}

	// Remove script object from inventory array.
	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (item == sub_item)
		{
			item_array->Remove_Index(i);
			SAFE_DELETE(item);
			return;
		}
	}

	Inventory_Space_Dirty = true;

	DBG_ASSERT(false);
}

void Profile::Unequip_Item(Item* skill)
{
	skill->equip_slot = -1;
	Item_Slots_Dirty = true;
}

void Profile::Equip_Item(Item* skill)
{
	// Already equipped.
	//if (skill->equip_slot >= 0)
	//	return;

	// Unequip old skill.
	Item* old_skill = Get_Item_Slot(skill->archetype->item_slot);
	if (old_skill != NULL)
		old_skill->equip_slot = -1;

	// Equip new skill.
	skill->equip_slot = skill->archetype->item_slot;
	skill->equip_time = Platform::Get()->Get_Timestamp();

	Item_Slots_Dirty = true;
}

Item* Profile::Get_Item_Slot(int slot)
{
	if (Item_Slots_Dirty)
	{
		for (int i = 0; i < ItemSlot::COUNT; i++)
		{
			Item_Slot_Items[i] = NULL;
		}

		CVMObject* item_array = Item_Array.Get().Get();
		for (int i = 0; i < item_array->Slot_Count(); i++)
		{
			Item* item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
			if (item->equip_slot >= 0)
			{
				Item_Slot_Items[item->equip_slot] = item;
			}
		}

		Item_Slots_Dirty = false;
	}

	return Item_Slot_Items[slot];
}

Skill* Profile::Unlock_Skill(SkillArchetype* archetype)
{
	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	vm->Get_Static_Context()->MetaData = this;
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* skill_class = vm->Find_Class("Skill");
	CVMObjectHandle new_skill = vm->New_Object(skill_class, true);

	Skill* item = reinterpret_cast<Skill*>(new_skill.Get()->Get_Meta_Data());
	item->archetype			= archetype;
	item->recieve_time		= Platform::Get()->Get_Timestamp();
	item->unique_id			= Get_Unique_ID(); 
	item->script_object		= new_skill;
	item->was_rolled_back	= false;

	// Add to inventory array.
	Skill_Array.Get().Get()->Add_Last(new_skill);
	
	// Invoke the On_Recieve event.
	CVMLinkedSymbol* archetype_class = vm->Find_Class("Skill_Archetype");
	CVMLinkedSymbol* onrecieve_event = vm->Find_Function(archetype_class, "On_Recieve", 2, "Profile", "Skill");
	CVMValue param_1 = Script_Object.Get();
	CVMValue param_2 = new_skill;
	CVMValue instance = item->archetype->script_object.Get();
	vm->Push_Parameter(param_1);
	vm->Push_Parameter(param_2);
	vm->Invoke(onrecieve_event, instance, false, false, false);

	DBG_LOG("Added skill '%s' to inventory.", S(archetype->name));

	return item;
}

bool Profile::Has_Skill(SkillArchetype* archetype)
{
	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());

		ProfileItemStack* add_stack = NULL;

		if (item->archetype == archetype)
		{
			return true;
		}

	}
	return false;
}

Skill* Profile::Get_Skill(SkillArchetype* archetype)
{
	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());

		ProfileItemStack* add_stack = NULL;

		if (item->archetype == archetype)
		{
			return item;
		}

	}
	return NULL;
}

int Profile::Get_Total_Used_Skill_Points()
{
	CVMObject* item_array = Skill_Array.Get().Get();
	int points = 0;

	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (!item->archetype->cannot_rollback)
		{
			points += item->archetype->cost;
		}
	}

	return points;
}

void Profile::Clear_Skills()
{
	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* sub_item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		SAFE_DELETE(sub_item);
	}

	item_array->Clear();
	item_array->Resize(0);
}

void Profile::Unequip_Skill(Skill* skill)
{
	skill->equip_slot = -1;
}

void Profile::Equip_Skill(Skill* skill, int slot)
{
	// Already equipped.
	//if (skill->equip_slot >= 0)
	//	return;

	// Unequip old skill.
	Skill* old_skill = Get_Skill_Slot(slot);
	if (old_skill != NULL)
		old_skill->equip_slot = -1;

	// Equip new skill.
	skill->equip_slot = slot;
	skill->equip_time = Platform::Get()->Get_Timestamp();
}

Skill* Profile::Get_Longest_Equipped_Skill()
{
	Skill* longest_equipped = NULL;

	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (item->equip_slot >= 0)
		{
			if (longest_equipped == NULL || item->equip_time < longest_equipped->equip_time)
			{
				longest_equipped = item;
			}
		}
	}

	return longest_equipped;
}

Skill* Profile::Get_Skill_Slot(int slot)
{
	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (item->equip_slot == slot)
		{
			return item;
		}
	}

	return NULL;
}

int Profile::Get_Next_Equip_Slot()
{
	// Try and find empty space.
	for (int i = 0; i < 3; i++)
	{
		if (Get_Skill_Slot(i) == NULL)
			return i;
	}

	// Find oldest equipped item.
	Skill* longest_equipped = Get_Longest_Equipped_Skill();

	if (longest_equipped != NULL)
		return longest_equipped->equip_slot;
	else
		return 0;
}

int Profile::Get_Reroll_Cost()
{
	return Get_Total_Used_Skill_Points() * ProfileManager::Get()->Get_Skill_Reroll_Cost();
}

void Profile::Reroll_Skills()
{
	Coins -= Get_Reroll_Cost();
	Skill_Points += Get_Total_Used_Skill_Points();

	// Rollback all skills that allow rollback, the reset persist (ususally instant-item ones to prevent exploits).
	CVMObject* item_array = Skill_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Skill* sub_item = reinterpret_cast<Skill*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype->cannot_rollback == false)
		{
			item_array->Remove_Index(i);
			i--;
			continue;
		}
		else
		{
			sub_item->was_rolled_back = true;
		}
	}

	{
		EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
		vm->Get_Static_Context()->MetaData = this;
		CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

		// Call rollback method to reset things.
		CVMLinkedSymbol* archetype_class = vm->Find_Class("Profile");
		CVMLinkedSymbol* event = vm->Find_Function(archetype_class, "On_Reroll_Skills", 0);
		CVMValue instance_value = Script_Object.Get();
		vm->Invoke(event, instance_value, false, false, false);
	}
}

bool Profile::Has_DLC(DLC* dlc)
{
	for (int i = 0; i < DLC_Array.Get().Get()->Slot_Count(); i++)
	{
		if (DLC_Array.Get().Get()->Get_Slot(i).int_value == dlc->id)
		{
			return true;
		}
	}
	return false;
}

void Profile::Apply_DLC(DLC* dlc)
{
	DBG_LOG("Upgrading profile '%s' with dlc '%s'.", this->Name.c_str(), dlc->name.c_str());

	EngineVirtualMachine* vm = GameEngine::Get()->Get_VM();
	vm->Get_Static_Context()->MetaData = dlc;
	CVMContextLock lock = vm->Set_Context(vm->Get_Static_Context());

	CVMLinkedSymbol* archetype_class = vm->Find_Class("DLC");
	CVMLinkedSymbol* event = vm->Find_Function(archetype_class, "On_Recieve", 1, "Profile");
	DBG_ASSERT(event != NULL);

	CVMValue instance_value = dlc->script_object.Get();
	CVMValue param1_value = this->Script_Object.Get();
	vm->Push_Parameter(param1_value);
	vm->Invoke(event, instance_value, false, false, false);

	// Add to DLC array.
	DLC_Array.Get().Get()->Add_Last(dlc->id);
}

bool Profile::Can_Combine_Item(Item* item_a, Item* item_b)
{
	// Can src->dst combine?
	for (int i = 0; i < item_a->archetype->combination_count; i++)
	{
		ItemCombination& combo = item_a->archetype->combinations[i];
		if (combo.other_group == item_b->archetype->combine_group ||
			combo.other_group == item_b->archetype->combine_name)
		{
			if (combo.combine_method == ItemCombineMethod::Repair)
			{
				return (item_a->equip_duration > 0.0f && !item_a->indestructable);
			}
			else
			{
				return true;
			}
		}
	}

	// Can dst->src combine?
	for (int i = 0; i < item_b->archetype->combination_count; i++)
	{
		ItemCombination& combo = item_b->archetype->combinations[i];
		if (combo.other_group == item_a->archetype->combine_group ||
			combo.other_group == item_a->archetype->combine_name)
		{
			if (combo.combine_method == ItemCombineMethod::Repair)
			{
				return (item_b->equip_duration > 0.0f && !item_b->indestructable);
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

void Profile::Combine_Item(Item* original_item_a, Item* original_item_b, Color custom_tint_color)
{
	Item* item_a = original_item_a;
	Item* item_b = original_item_b;

	// Do we need to invert a/b items?
	for (int i = 0; i < item_b->archetype->combination_count; i++)
	{
		ItemCombination& combo = item_b->archetype->combinations[i];
		if (combo.other_group == item_a->archetype->combine_group ||
			combo.other_group == item_a->archetype->combine_name)
		{
			item_a = item_b;
			item_b = original_item_a;
			break;
		}
	}
	
	DBG_LOG("Combining '%s' and '%s'.", item_a->archetype->base_name.c_str(), item_b->archetype->base_name.c_str());
	
	// Do combine!
	for (int i = 0; i < item_a->archetype->combination_count; i++)
	{
		ItemCombination& combo = item_a->archetype->combinations[i];
		if (combo.other_group == item_b->archetype->combine_group ||
			combo.other_group == item_b->archetype->combine_name)
		{
			switch (combo.combine_method)
			{
			case ItemCombineMethod::Attach:
				{
					// Remove previously attached item.
					std::vector<Item*> attached = Get_Attached_Items(item_a);
					for (std::vector<Item*>::iterator iter = attached.begin(); iter != attached.end(); iter++)
					{
						Remove_Item(*iter);
					}

					item_b->attached_to_id = item_a->unique_id;
					Inventory_Space_Dirty = true;

					break;
				}
			case ItemCombineMethod::Merge:
				{
					ItemArchetype* arch = ItemManager::Get()->Find_Archetype_By_Combo_Name(combo.result_item.c_str());
					DBG_ASSERT(arch != NULL);

					Remove_Item(item_a);
					Remove_Item(item_b);
					Add_Item(arch);
					break;
				}
			case ItemCombineMethod::Tint:
				{
					if (item_a->archetype->use_custom_color || item_b->archetype->use_custom_color)
					{
						item_a->primary_color = custom_tint_color;
					}
					else
					{
						item_a->primary_color = item_b->archetype->default_tint;
					}

					// Indestructable paints do not need removing.
					if (!item_b->indestructable)
					{
						Remove_Item(item_b, true);
					}

					break;
				}
			case ItemCombineMethod::Repair:
				{
					item_a->equip_duration = 0.0f;

					// Indestructable items do not need removing.
					// Update: Yes they fucking do.
					//if (!item_b->indestructable)
					{
						Remove_Item(item_b, true);
					}

					break;
				}
			}
			break;
		}
	}
}

std::vector<Item*> Profile::Get_Attached_Items(Item* item)
{
	std::vector<Item*> result;

	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->attached_to_id >= 0 && sub_item->attached_to_id == item->unique_id)
		{
			result.push_back(sub_item);
		}
	}

	return result;
}

Item* Profile::Get_Item(ItemArchetype* type)
{
	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type)
		{
			return sub_item;
		}
	}
	return NULL;
}

Item* Profile::Get_Preferred_Consume_Item(ItemArchetype* type)
{
	CVMObject* item_array = Item_Array.Get().Get();

	// First - none-inventory, destructable.
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type && 
			!sub_item->was_inventory_drop &&
			!sub_item->indestructable)
		{
			return sub_item;
		}
	}

	// Second - none-inventory, non-destructable.
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type &&
			!sub_item->was_inventory_drop &&
			sub_item->indestructable)
		{
			return sub_item;
		}
	}

	// Third - Fuck it, use anything.
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type)
		{
			return sub_item;
		}
	}

	return NULL;
}

bool Profile::Has_Item(ItemArchetype* type)
{
	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type)
		{
			return true;
		}
	}
	return false;
}

int Profile::Get_Item_Count(ItemArchetype* type)
{
	int total = 0;

	CVMObject* item_array = Item_Array.Get().Get();
	for (int i = 0; i < item_array->Slot_Count(); i++)
	{
		Item* sub_item = reinterpret_cast<Item*>(item_array->Get_Slot(i).object_value.Get()->Get_Meta_Data());
		if (sub_item->archetype == type)
		{
			total++;
		}
	}

	return total;
}

void Profile::Level_Up()
{
	int next_level_xp = ProfileManager::Get()->Get_Level_XP(Level + 1);

	Level++;
	Level_XP = Max(0, Level_XP - next_level_xp);
	Skill_Points += ProfileManager::Get()->Get_Skill_Points_Per_Level();

	Game::Get()->Queue_Save();

	ScriptEventListener::Fire_On_Level_Up();

	DBG_LOG("%s leved up to level %i, %i rollover xp. Has %i skill points.", Name.c_str(), Level, Level_XP, Skill_Points);
}

int Profile::Give_XP(int amount)
{
	bool leveled_up = false;

	Level_XP += amount;
	while (true)
	{
		int next_level_xp = ProfileManager::Get()->Get_Level_XP(Level + 1);
//		DBG_LOG("Level:%i Level-XP:%i Next:%i", Level, Level_XP, next_level_xp);
		if (Level_XP >= next_level_xp && Level < ProfileManager::Get()->Get_Level_Cap())
		{
			Level_Up();
			leveled_up = true;
		}
		else
		{
			break;
		}
	}

	return leveled_up;
}

int Profile::Give_Fractional_XP(float amount)
{
	return Give_XP((int)(ProfileManager::Get()->Get_Level_XP(Level + 1) * amount));
}

int Profile::Get_Unique_ID()
{
	return Unique_ID_Counter++;
}

float Profile::Get_Stat_Multiplier(Profile_Stat::Type stat)
{
	CVMObject* arr = Stat_Multipliers.Get().Get();
	if (arr == NULL)
	{
		return 1.0f;
	}
	return arr->Get_Slot((int)stat).float_value;
}

void Profile::Set_Stat_Multiplier(Profile_Stat::Type stat, float value)
{
	Stat_Multipliers.Get().Get()->Get_Slot((int)stat).float_value = value;
}

float Profile::Get_Item_Stat_Multiplier(Profile_Stat::Type stat)
{
	float multiplier = 1.0f;

	if (stat == Profile_Stat::Health_Regen || 
		stat == Profile_Stat::Ammo_Regen || 
		stat == Profile_Stat::Wallet_Interest)
	{
		multiplier = 0.0f;
	}

	for (int i = 0; i < ItemSlot::All; i++)
	{
		Item* item = Get_Item_Slot(i);
		if (item == NULL || !item->was_inventory_drop)
		{
			continue;
		}

		if (!item->item_stats_cached)
		{
			item->cached_item_stats = UniqueItemStats(item);
			item->item_stats_cached = true;
		}

		if (stat == Profile_Stat::Health_Regen || 
			stat == Profile_Stat::Ammo_Regen || 
			stat == Profile_Stat::Wallet_Interest)
		{
			multiplier += item->cached_item_stats.Stats[stat];
		}
		else
		{
			multiplier *= item->cached_item_stats.Stats[stat];
		}
	}

	//if (multiplier != 0.0f && multiplier != 1.0f)
	//{
	//	DBG_LOG("STAT[%i] = %.3f", stat, multiplier);
	//}

	return multiplier;
}

void Profile::Tick(const FrameTime& time)
{
	float seconds = time.Get_Delta_Seconds();
	float intrest_per_hour = Coins * Get_Stat_Multiplier(Profile_Stat::Wallet_Interest);
	float intrest_per_frame = ((intrest_per_hour / 60.0f) / 60.0f) * seconds;

	Coins_Interest += intrest_per_frame;

	int whole = (int)Coins_Interest;
	if (whole > 0)
	{
		Coins += whole;
		Coins_Interest -= whole;
	}

	// Hard cap to make sure coins/skills don't get messed up at any point.
	Coins = Min(Coins, Wallet_Size);
	Skill_Points = Min(Skill_Points, 100);
}

void Profile::Check_Unlocks()
{
	// Go through all item archetypes and see if any new ones have unlocked.
	std::vector<ItemArchetype*> items = ItemManager::Get()->Get_Archetypes();

	for (std::vector<ItemArchetype*>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		ItemArchetype* item = *iter;
		if (Is_Item_Unlocked(item))
		{
			continue;
		}

		bool bUnlocked = false;

		switch (item->unlock_critera)
		{
		case ItemUnlockCriteria::Required_Rank:
			{
				bUnlocked = (this->Level >= item->unlock_critera_threshold);
				break;
			}
		case ItemUnlockCriteria::Ailments_From_Weapon:
			{
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);
				bUnlocked = (Get_Item_Stat(unlock_by, ProfileItemStat::Ailments_Inflicted) >= item->unlock_critera_threshold);
				break;
			}
		case ItemUnlockCriteria::Damage_With_Weapon:
			{
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);
				float amt = fabs(Get_Item_Stat(unlock_by, ProfileItemStat::Damage_Inflicted));
				bUnlocked = (amt >= fabs(item->unlock_critera_threshold));
				break;
			}
		case ItemUnlockCriteria::Kills_With_Weapon:
			{
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);
				bUnlocked = (Get_Item_Stat(unlock_by, ProfileItemStat::Enemies_Killed) >= item->unlock_critera_threshold);
				break;
			}
		}

		if (bUnlocked && item->is_buyable)
		{
			DBG_LOG("Unlocked item '%s' in shop.", item->base_name.c_str());

			int item_id = item->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;
			Unlocked_Item_Array.Get().Get()->Add_Last(item_id);

			Item_Unlock_Events.push_back(item);
		}
	}

	// Go through all item archetypes and see if any new ones have unlocked.
	std::vector<SkillArchetype*> skills = SkillManager::Get()->Get_Archetypes();

	for (std::vector<SkillArchetype*>::iterator iter = skills.begin(); iter != skills.end(); iter++)
	{
		SkillArchetype* item = *iter;
		if (Is_Skill_Unlocked(item))
		{
			continue;
		}

		bool bUnlocked = false;

		switch (item->unlock_critera)
		{
			case ItemUnlockCriteria::Required_Rank:
			{
				bUnlocked = (this->Level >= item->unlock_critera_threshold);
				break;
			}
			case ItemUnlockCriteria::Ailments_From_Weapon:
			{
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);
				bUnlocked = (Get_Item_Stat(unlock_by, ProfileItemStat::Ailments_Inflicted) >= item->unlock_critera_threshold);
				break;
			}
			case ItemUnlockCriteria::Damage_With_Weapon:
			{
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);

				float amt = fabs(Get_Item_Stat(unlock_by, ProfileItemStat::Damage_Inflicted));
				bUnlocked = (amt >= fabs(item->unlock_critera_threshold));

				break;
			}
			case ItemUnlockCriteria::Kills_With_Weapon:
			{				
				ItemArchetype* unlock_by = ItemManager::Get()->Find_Archetype(item->unlock_critera_item->symbol->unique_id);
				bUnlocked = (Get_Item_Stat(unlock_by, ProfileItemStat::Enemies_Killed) >= item->unlock_critera_threshold);
				break;	
			}
		}

		if (bUnlocked)
		{
			DBG_LOG("Unlocked skill '%s' in shop.", item->name.c_str());

			int item_id = item->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;
			Unlocked_Skill_Array.Get().Get()->Add_Last(item_id);

			Skill_Unlock_Events.push_back(item);
		}
	}
}

bool Profile::Is_Item_Unlocked(ItemArchetype* item)
{
	int item_id = item->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Unlocked_Item_Array.Get().Get()->Slot_Count(); i++)
	{
		if (Unlocked_Item_Array.Get().Get()->Get_Slot(i).int_value == item_id)
		{
			return true;
		}
	}

	return false;
}

bool Profile::Is_Skill_Unlocked(SkillArchetype* item)
{
	int item_id = item->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Unlocked_Skill_Array.Get().Get()->Slot_Count(); i++)
	{
		if (Unlocked_Skill_Array.Get().Get()->Get_Slot(i).int_value == item_id)
		{
			return true;
		}
	}

	return false;
}

float Profile::Get_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	int item_id = archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Profile_Item_Stats_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Item_Stats_Array.Get().Get()->Get_Slot(i).object_value.Get();
		if (obj->Get_Slot(0).int_value == item_id &&
			obj->Get_Slot(1).int_value == stat)
		{
			return obj->Get_Slot(2).float_value;
		}
	}

	return 0.0f;
}

void Profile::Set_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat, float amount)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	int item_id = archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Profile_Item_Stats_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Item_Stats_Array.Get().Get()->Get_Slot(i).object_value.Get();
		if (obj->Get_Slot(0).int_value == item_id &&
			obj->Get_Slot(1).int_value == stat)
		{
			obj->Get_Slot(2).float_value = amount;
			return;
		}
	}

	CVMObject* obj = vm->New_Object(Profile_Item_Stat_Class, false).Get();
	obj->Get_Slot(0).int_value = item_id;
	obj->Get_Slot(1).int_value = stat;
	obj->Get_Slot(2).float_value = amount;

	Profile_Item_Stats_Array.Get().Get()->Add_Last(obj);

//	DBG_LOG("Set stat %i for weapon %s by %f", stat, archetype->base_name.c_str(), amount);
}

void Profile::Increment_Item_Stat(ItemArchetype* archetype, ProfileItemStat::Type stat, float amount)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	int item_id = archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Profile_Item_Stats_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Item_Stats_Array.Get().Get()->Get_Slot(i).object_value.Get();
		if (obj->Get_Slot(0).int_value == item_id &&
			obj->Get_Slot(1).int_value == stat)
		{
			obj->Get_Slot(2).float_value += amount;

			if (stat == (int)ProfileItemStat::Damage_Inflicted)
			{
				//DBG_LOG("Inc stat %i for weapon %s by %f to %f", stat, archetype->base_name.c_str(), amount, obj->Get_Slot(2).float_value);
			}
			return;
		}
	}

	CVMObject* obj = vm->New_Object(Profile_Item_Stat_Class, false).Get();
	obj->Get_Slot(0).int_value = item_id;
	obj->Get_Slot(1).int_value = stat;
	obj->Get_Slot(2).float_value = amount;

	Profile_Item_Stats_Array.Get().Get()->Add_Last(obj);

	//DBG_LOG("New stat %i for weapon %s by %f", stat, archetype->base_name.c_str(), amount);
}

float Profile::Get_Skill_Energy(SkillArchetype* archetype)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	int item_id = archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Profile_Skill_Energies_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Skill_Energies_Array.Get().Get()->Get_Slot(i).object_value.Get();
		if (obj->Get_Slot(0).int_value == item_id)
		{
			return obj->Get_Slot(2).float_value;
		}
	}

	CVMObject* obj = vm->New_Object(Profile_Skill_Energy_Class, false).Get();
	obj->Get_Slot(0).int_value = item_id;
	obj->Get_Slot(1).int_value = archetype->energy_type;
	obj->Get_Slot(2).float_value = 0.0f;

	Profile_Skill_Energies_Array.Get().Get()->Add_Last(obj);

	return 0.0f;
}

void Profile::Set_Item_Stat(SkillArchetype* archetype, float amount)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	int item_id = archetype->script_object.Get().Get()->Get_Symbol()->symbol->unique_id;

	for (int i = 0; i < Profile_Skill_Energies_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Skill_Energies_Array.Get().Get()->Get_Slot(i).object_value.Get();
		if (obj->Get_Slot(0).int_value == item_id)
		{
			obj->Get_Slot(2).float_value = amount;
			return;
		}
	}

	CVMObject* obj = vm->New_Object(Profile_Skill_Energy_Class, false).Get();
	obj->Get_Slot(0).int_value = item_id;
	obj->Get_Slot(1).int_value = archetype->energy_type;
	obj->Get_Slot(2).float_value = amount;

	Profile_Skill_Energies_Array.Get().Get()->Add_Last(obj);
}

void Profile::Increment_Skill_Energies(SkillEnergyType::Type type, float amount)
{
	CVirtualMachine* vm = GameEngine::Get()->Get_VM();

	for (int i = 0; i < Profile_Skill_Energies_Array.Get().Get()->Slot_Count(); i++)
	{
		CVMObject* obj = Profile_Skill_Energies_Array.Get().Get()->Get_Slot(i).object_value.Get();
 		if (obj->Get_Slot(1).int_value == (int)type)
		{
			obj->Get_Slot(2).float_value += amount;
		}
	}
}

Item* Profile::Combo_Create(ItemArchetype* result_item)
{
	std::vector<Item*> items = Get_Items();
	std::vector<ItemArchetype*> resources;

	for (std::vector<Item*>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		Item* item = *iter;
		resources.push_back(item->archetype);
	}
	
	if (!Can_Combo_Create_Internal(result_item, "", resources))
	{
		return NULL;
	}

	// Remove all resources from item list. The items left in the list are the ones
	// we need to combine to create the item.
	for (std::vector<ItemArchetype*>::iterator iter = resources.begin(); iter != resources.end(); iter++)
	{
		ItemArchetype* archetype = *iter;

		bool bFound = true;

		for (std::vector<Item*>::iterator itemIter = items.begin(); itemIter != items.end(); itemIter++)
		{
			Item* item = *itemIter;
			if (item->archetype == archetype)
			{
				items.erase(itemIter);
				bFound = true;
				break;
			}
		}

		DBG_ASSERT(bFound);
	}

	// Remove items from inventory.
	for (std::vector<Item*>::iterator itemIter = items.begin(); itemIter != items.end(); itemIter++)
	{
		Item* item = *itemIter;
		Remove_Item(item, true);
	}

	return Add_Item(result_item);
}

bool Profile::Can_Combo_Create(ItemArchetype* result_item)
{
	std::vector<Item*> items = Get_Items();
	std::vector<ItemArchetype*> resources;

	for (std::vector<Item*>::iterator iter = items.begin(); iter != items.end(); iter++)
	{
		Item* item = *iter;
		resources.push_back(item->archetype);
	}

	return Can_Combo_Create_Internal(result_item, "", resources);
}

bool Profile::Can_Combo_Create_Internal(ItemArchetype* result_item, std::string result_item_group, std::vector<ItemArchetype*>& resources)
{
	std::vector<ItemArchetype*> archetypes = ItemManager::Get()->Get_Archetypes();
	
	// Now try and perform combos to create the item.
	for (std::vector<ItemArchetype*>::iterator iter = archetypes.begin(); iter != archetypes.end(); iter++)
	{
		ItemArchetype* archetype = *iter;

		for (int i = 0; i < archetype->combination_count; i++)
		{
			if (archetype->combinations[i].combine_method == ItemCombineMethod::Merge)
			{
				ItemArchetype* result_item_arch = ItemManager::Get()->Find_Archetype_By_Combo_Name(archetype->combinations[i].result_item.c_str());
				if ((result_item_group != "" && (result_item_arch->combine_group == result_item_group || result_item_arch->combine_name == result_item_group)) || (result_item_arch == result_item))
				{				
					ItemArchetype* source_resource = NULL;
					ItemArchetype* target_resource = NULL;

					// Do we have the source item + combo item already? Sweet, no need to do nested combines.
					for (std::vector<ItemArchetype*>::iterator resIter = resources.begin(); resIter != resources.end(); resIter++)
					{
						ItemArchetype* resource = *resIter;
						if (source_resource == NULL && (archetype == resource))
						{
							source_resource = resource;
						}
						else if (target_resource == NULL && (archetype->combinations[i].other_group == resource->combine_group || archetype->combinations[i].other_group == resource->combine_name))
						{
							target_resource = resource;
						}
					}

					// We have resources!
					if (source_resource && target_resource)
					{
						// Remove the resources are using.
						resources.erase(std::find(resources.begin(), resources.end(), source_resource));
						resources.erase(std::find(resources.begin(), resources.end(), target_resource));

						return true;
					}

					// Nope, guess we'll have to do some nested combos.
					else
					{
						std::vector<ItemArchetype*> sub_resources = resources;

						if (source_resource) resources.erase(std::find(resources.begin(), resources.end(), source_resource));
						if (target_resource) resources.erase(std::find(resources.begin(), resources.end(), target_resource));

						if (!source_resource)
						{
							if (!Can_Combo_Create_Internal(archetype, "", sub_resources))
							{
								// We won't be using this combo then :(
								continue;
							}
						}

						if (!target_resource)
						{
							if (!Can_Combo_Create_Internal(NULL, archetype->combinations[i].other_group, sub_resources))
							{
								// We won't be using this combo then :(
								continue;
							}
						}

						// Update the resource list to what it would be after those sub items were generated.
						resources = sub_resources;
						
						return true;
					}
				}
			}
		}
	}

	return false;
}
