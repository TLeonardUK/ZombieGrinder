// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.grenade.weapon_turret;

public class Item_Turret : Item_Archetype
{
	Item_Turret()
	{
		Name 					= "#item_turret_name";
		Description 			= "#item_turret_description";
		Category				= "#item_turret_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 19;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV100";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_turret";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_idle";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
        Override_Inventory_ID_Name = "Item_Turret_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Turret_Upgrade_Tree));
	
		Weapon_Type				= typeof(Weapon_Turret);
		
		Combine_Name			= "Turret";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
