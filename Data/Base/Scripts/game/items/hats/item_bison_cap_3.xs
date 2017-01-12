// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Bison_Cap_3 : Item_Archetype
{
	Item_Bison_Cap_3()
	{
		Name 					= "#item_bison_cap_3_name";
		Description 			= "#item_bison_cap_3_description";
		Category				= "#item_bison_cap_3_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 80000;
		Unlock_Criteria_Threshold			= 32;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= false;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(220, 60, 70, 255);
		
		Icon_Animation			= "item_hat_bison_cap_3";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Bison Bronze";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
