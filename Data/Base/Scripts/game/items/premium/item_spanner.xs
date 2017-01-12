// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_freeze_ray;

public class Item_Spanner : Item_Archetype
{
	Item_Spanner()
	{
		Name 					= "#item_spanner_name";
		Description 			= "#item_spanner_description";
		Category				= "#item_spanner_category";
		
		Slot					= Item_Slot.All;
		Cost					= 5000;
		Unlock_Criteria_Threshold			= 0;
		
		Is_Stackable			= true;
		Max_Stack 				= 99;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV100";
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_premium_gem_removal_kit";
		Icon_Offset				= Vec2(12, 12);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Spanner";
		Combine_Group			= "Spanners";
		Combinations 			= new Item_Combination[0];
	}
}
