// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Heroic_Hood : Item_Archetype
{
	Item_Heroic_Hood()
	{
		Name 					= "#item_heroic_hood_name";
		Description 				= "#item_heroic_hood_description";
		Category				= "#item_heroic_hood_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 50000;
		Unlock_Criteria_Threshold			= 26;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= false;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(157, 187, 97, 255);
		
		Icon_Animation			= "item_hat_heroic_hood";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Heroic Hood";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
