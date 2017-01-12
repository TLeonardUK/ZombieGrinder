// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_laser_cannon;

public class Item_Laser_Cannon : Item_Archetype
{
	Item_Laser_Cannon()
	{
		Name 					= "#item_laser_cannon_name";
		Description 			= "#item_laser_cannon_description";
		Category				= "#item_laser_cannon_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 150000;
		Unlock_Criteria_Threshold			= 20;
		
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
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_laser_cannon";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_lasercannon";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Laser_Cannon);
		
        Override_Inventory_ID_Name = "Item_Laser_Cannon_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Laser_Cannon_Upgrade_Tree));
		
		Combine_Name			= "Laser Cannon";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
