// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Halo : Item_Archetype
{
	Item_Halo()
	{
		Name 					= "#item_halo_name";
		Description 			= "#item_halo_description";
		Category				= "#item_halo_category";
		
		Slot					= Item_Slot.Accessory;
		Cost					= 50000;
		
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= true;
		Is_Sellable				= false;
		Is_Tradable				= false;
		Is_Dropable				= false;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 230, 107, 255);
		
		Icon_Animation			= "item_hat_halo";
		Icon_Offset				= Vec2(32, 23);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Developer Halo";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
