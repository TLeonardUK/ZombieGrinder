// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Green_Gem_1 : Item_Archetype
{
	Item_Green_Gem_1()
	{
		Name 					= "#item_green_gem_1_name";
		Description 			= "#item_green_gem_1_description";
		Category				= "#item_green_gem_1_category";
		
		Slot					= Item_Slot.All;
		Cost					= 20000;
		Unlock_Criteria_Threshold			= 0;
		
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
		Premium_Price			= "1;VLV25";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(34, 177, 76, 255);
		
		Icon_Animation			= "item_gem_damage_1";
		Icon_Offset				= Vec2(8, 8);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Crappy Emerald";
		Combine_Group			= "Gems";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Crappy Emerald", Item_Combine_Method.Merge, "Decent Emerald");
	}
}
