// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Purple_Gem_5 : Item_Archetype
{
	Item_Purple_Gem_5()
	{
		Name 					= "#item_purple_gem_5_name";
		Description 			= "#item_purple_gem_5_description";
		Category				= "#item_purple_gem_5_category";
		
		Slot					= Item_Slot.All;
		Cost					= 320000;
		Unlock_Criteria_Threshold			= 20;
		
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
		Premium_Price			= "1;VLV400";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(192, 106, 221, 255);
		
		Icon_Animation			= "item_gem_ammocapacity_5";
		Icon_Offset				= Vec2(8, 8);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Legit Amethyst";
		Combine_Group			= "Gems";
		Combinations 			= new Item_Combination[0];
	}
}
