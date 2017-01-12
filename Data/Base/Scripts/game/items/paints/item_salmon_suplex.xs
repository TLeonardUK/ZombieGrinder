// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Salmon_Suplex : Item_Archetype
{
	Item_Salmon_Suplex()
	{
		Name 					= "#item_salmon_suplex_name";
		Description 			= "#item_salmon_suplex_description";
		Category				= "#item_salmon_suplex_category";
		
		Slot					= Item_Slot.All;
		Cost					= 5000;
		
		Is_Stackable			= true;
		Max_Stack 				= 5;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV25";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(238, 139, 134, 255);
		
		Icon_Animation			= "item_paint_bucket";
		Icon_Offset				= Vec2(16, 16);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Salmon Suplex";
		Combine_Group			= "Paints";
		Combinations 			= new Item_Combination[0];
	}
}
