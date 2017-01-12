// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Deus_Ex : Item_Archetype
{
	Item_Deus_Ex()
	{
		Name 					= "#item_deus_ex_name";
		Description 			= "#item_deus_ex_description";
		Category				= "#item_deus_ex_category";
		
		Slot					= Item_Slot.Accessory;
		Cost					= 50000;
		Unlock_Criteria_Threshold			= 20;
		
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
		
		Post_Process_FX_Type	= typeof(Deus_Ex_Post_Process_FX);
		 
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 233, 0, 255);
		
		Icon_Animation			= "item_accessory_deus_ex";
		Icon_Offset				= Vec2(32, 40);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Augmented Shades";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
