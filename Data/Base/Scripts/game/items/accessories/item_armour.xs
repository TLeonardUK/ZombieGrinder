// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Armour : Item_Archetype
{
	Item_Armour()
	{
		Name 					= "#item_armour_name";
		Description 			= "#item_armour_description";
		Category				= "#item_armour_category";
		
		Slot					= Item_Slot.Accessory;
		Cost					= 35000;
		Unlock_Criteria_Threshold			= 9;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_accessory_armour";
		Icon_Offset				= Vec2(9, 10);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;

        Initial_Armour_Amount   = 100;
		
		Combine_Name			= "Armour";
		Combine_Group			= "Armour";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
