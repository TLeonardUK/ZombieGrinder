// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Pink_Gem_2 : Item_Archetype
{
	Item_Pink_Gem_2()
	{
		Name 					= "#item_pink_gem_2_name";
		Description 			= "#item_pink_gem_2_description";
		Category				= "#item_pink_gem_2_category";
		
		Slot					= Item_Slot.All;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 5;
		
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
		Premium_Price			= "1;VLV50";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 102, 171, 255);
		
		Icon_Animation			= "item_gem_price_2";
		Icon_Offset				= Vec2(8, 8);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Decent Opal";
		Combine_Group			= "Gems";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Decent Opal", Item_Combine_Method.Merge, "Neat Opal");
	}
}
