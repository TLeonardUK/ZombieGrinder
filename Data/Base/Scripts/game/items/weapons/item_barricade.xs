// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.grenade.weapon_turret;

public class Item_Barricade : Item_Archetype
{
	Item_Barricade()
	{
		Name 					= "#item_barricade_name";
		Description 			= "#item_barricade_description";
		Category				= "#item_barricade_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 15;
		
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
		
		Icon_Animation			= "item_weapon_barricade";
		Icon_Offset				= Vec2(16, 20);
		
		Body_Animation			= "avatar_body_idle";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
        Override_Inventory_ID_Name = "Item_Barricade_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Barricade_Upgrade_Tree));
	
		Weapon_Type				= typeof(Weapon_Barricade);
		
		Combine_Name			= "Barricade";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
