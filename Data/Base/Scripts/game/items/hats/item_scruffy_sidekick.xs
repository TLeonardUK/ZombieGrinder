// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Scruffy_Sidekick : Item_Archetype
{
	Item_Scruffy_Sidekick()
	{
		Name 					= "#item_scruffy_sidekick_name";
		Description 			= "#item_scruffy_sidekick_description";
		Category				= "#item_scruffy_sidekick_category";
		
		Slot					= Item_Slot.Head;
		Cost					= 20000;
		Unlock_Criteria_Threshold			= 7;
		
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
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(140, 90, 50, 255);
		
		Icon_Animation			= "item_hat_sidekick";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Scruffy Sidekick";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
