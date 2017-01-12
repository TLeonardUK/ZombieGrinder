// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_love_cannon;

public class Item_Love_Cannon : Item_Archetype
{
	Item_Love_Cannon()
	{
		Name 					= "#item_love_cannon_name";
		Description 			= "#item_love_cannon_description";
		Category				= "#item_love_cannon_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 20000;
		Unlock_Criteria_Threshold			= 13;
		
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
		
		Icon_Animation			= "item_weapon_love_cannon";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_lovecannon";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Love_Cannon);
		
        Override_Inventory_ID_Name = "Item_Love_Cannon_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Love_Cannon_Upgrade_Tree));
		
		Combine_Name			= "Love Cannon";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
