// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Pumpkin : Item_Archetype
{
	Item_Pumpkin()
	{
		Name 					= "#item_pumpkin_name";
		Description 			= "#item_pumpkin_description";
		Category				= "#item_pumpkin_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 30000;
		
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 100, 0, 255);
		
		Icon_Animation			= "item_hat_pumpkin";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Max_Durability			= -1.0f; // Indestructable without the actual indestructable tag. Less confusing maybe kinda, blah.
		
		Combine_Name			= "Pumpkin!";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
