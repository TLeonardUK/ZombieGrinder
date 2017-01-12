// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_freeze_ray;

public class Item_Freeze_Ray : Item_Archetype
{
	Item_Freeze_Ray()
	{
		Name 					= "#item_freeze_ray_name";
		Description 			= "#item_freeze_ray_description";
		Category				= "#item_freeze_ray_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 30000;
		Unlock_Criteria_Threshold			= 12;
		
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
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_freeze_ray";
		Icon_Offset				= Vec2(12, 10);
		
		Body_Animation			= "avatar_body_freezeray";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Freeze_Ray);
		
        Override_Inventory_ID_Name = "Item_Freeze_Ray_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Freeze_Ray_Upgrade_Tree));
		
		Combine_Name			= "Freeze Ray";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
