// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_10000_Coins : Item_Archetype
{
	Item_10000_Coins()
	{
		Name 					= "[UNUSED] 10000 Coins";
		Description 			= "[UNUSED] 10000 Coins";
		Category				= "[UNUSED] 10000 Coins";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 200000;
		
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_accessory_10000_coins";
		Icon_Offset				= Vec2(31, 34);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Starting Coins x 10000";
		Combine_Group			= "Coins";
		Combinations 			= new Item_Combination[0];
	}
}
