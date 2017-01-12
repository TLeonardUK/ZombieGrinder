// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Gameboy : Item_Archetype
{
	Item_Gameboy()
	{
		Name 					= "#item_gameboy_name";
		Description 			= "#item_gameboy_description";
		Category				= "#item_gameboy_category";
		
		Slot					= Item_Slot.Accessory;
		Cost					= 50000;
		Unlock_Criteria_Threshold			= 2;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Post_Process_FX_Type	= typeof(Gameboy_Post_Process_FX);
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(0, 255, 0, 255);
		
		Icon_Animation			= "item_accessory_gameboy";
		Icon_Offset				= Vec2(32, 40);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Retrospecs";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
