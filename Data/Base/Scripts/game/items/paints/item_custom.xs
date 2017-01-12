// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Custom : Item_Archetype
{
	Item_Custom()
	{
		Name 					= "#item_custom_paint_name";
		Description 			= "#item_custom_paint_description";
		Category				= "#item_custom_paint_category";
		
		Slot					= Item_Slot.All;
		Cost					= 35000;
		Unlock_Criteria_Threshold			= 35;
		
		Is_Stackable			= true;
		Max_Stack 				= 5;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV25";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_paint_bucket";
		Icon_Offset				= Vec2(16, 16);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Custom Paint";
		Combine_Group			= "Paints";
		Use_Custom_Color		= true;
		Combinations 			= new Item_Combination[0];
	}
}
