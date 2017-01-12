// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.grenade.weapon_tripmine;

public class Item_Tripmine : Item_Archetype
{
	Item_Tripmine()
	{
		Name 					= "#item_tripmine_name";
		Description 			= "#item_tripmine_description";
		Category				= "#item_tripmine_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 30000;
		
		Is_Stackable			= false;
		Is_Buyable				= false;//true; // Waiting on new sprites.
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;//true;
		Is_Premium_Only			= false;
		Premium_Price			= "";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_tripmine";
		Icon_Offset				= Vec2(10, 10);
		
		Body_Animation			= "avatar_body_idle";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Tripmine);
		
		Combine_Name			= "Tripmine";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
