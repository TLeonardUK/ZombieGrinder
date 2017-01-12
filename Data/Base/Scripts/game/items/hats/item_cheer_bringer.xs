// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Cheer_Bringer : Item_Archetype
{
	Item_Cheer_Bringer()
	{
		Name 					= "#item_cheer_bringer_name";
		Description 			= "#item_cheer_bringer_description";
		Category				= "#item_cheer_bringer_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 40000;
		 
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= false;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "";
		 
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(237, 28, 36, 255);
		
		Icon_Animation			= "item_hat_cheer_bringer";
		Icon_Offset				= Vec2(32, 36);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Cheer Bringer";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
