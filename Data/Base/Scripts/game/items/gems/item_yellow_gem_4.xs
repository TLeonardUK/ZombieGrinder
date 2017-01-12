// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Yellow_Gem_4 : Item_Archetype
{
	Item_Yellow_Gem_4()
	{
		Name 					= "#item_yellow_gem_4_name";
		Description 			= "#item_yellow_gem_4_description";
		Category				= "#item_yellow_gem_4_category";
		
		Slot					= Item_Slot.All;
		Cost					= 160000;
		Unlock_Criteria_Threshold			= 15;
		
		Is_Stackable			= true;
		Max_Stack 				= 99;
		Is_Buyable				= false;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV200";
		
		Rarity					= Item_Rarity.Ultra_Rare;
		
		Default_Tint			= Vec4(255, 201, 36, 255);
		
		Icon_Animation			= "item_gem_rof_4";
		Icon_Offset				= Vec2(8, 8);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Sweet Topaz";
		Combine_Group			= "Gems";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Sweet Topaz", Item_Combine_Method.Merge, "Legit Topaz");
	}
}
