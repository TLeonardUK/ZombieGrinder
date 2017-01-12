// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Large_Donator : Item_Archetype
{
	Item_Large_Donator()
	{
		Name 					= "[UNUSED] Large Donator";
		Description 			= "[UNUSED] Large Donator";
		Category				= "[UNUSED] Large Donator";
		
		Slot					= Item_Slot.All;
		Cost					= 0; 
		
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= false;
		Is_Sellable				= false;
		Is_Tradable				= true;
		Is_Dropable				= false;
		Is_PVP_Usable			= false;
		Is_Inventory_Droppable	= false;
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_premium_large_donator";
		Icon_Offset				= Vec2(8, 8);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Large Donator Version";
		Combine_Group			= "Premium";
		Combinations 			= new Item_Combination[0];
	}
}
