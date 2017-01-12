// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Fred_Head : Item_Archetype
{
	Item_Fred_Head()
	{
		Name 					= "#item_fred_head_name";
		Description 			= "#item_fred_head_description";
		Category				= "#item_fred_head_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 30;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(150, 140, 100, 255);
		
		Icon_Animation			= "item_hat_fred_head";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Fred Head";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
