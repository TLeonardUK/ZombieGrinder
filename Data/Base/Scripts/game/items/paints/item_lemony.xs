// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Lemony : Item_Archetype
{
	Item_Lemony()
	{
		Name 					= "#item_lemony_name";
		Description 			= "#item_lemony_description";
		Category				= "#item_lemony_category";
		
		Slot					= Item_Slot.All;
		Cost					= 5000;
		
		Is_Stackable			= true;
		Max_Stack 				= 5;
		Is_Buyable				= false; // Disabled for now.
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV25";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(231, 181, 59, 255);
		
		Icon_Animation			= "item_paint_bucket";
		Icon_Offset				= Vec2(16, 16);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Lemony Yellow";
		Combine_Group			= "Paints";
		Combinations 			= new Item_Combination[0];
	}
}
