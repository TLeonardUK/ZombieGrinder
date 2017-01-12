// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Example_Hat : Item_Archetype
{
	Item_Example_Hat()
	{
		Name 					= "Example Hat";
		Description 			= "An example of how to make a hat through the mod tools!";
		Category				= "Hats";
		
		Slot					= Item_Slot.Head;
		Cost					= 1000;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(120, 70, 60, 255);
		
		Icon_Animation			= "item_example_hat";
		Icon_Offset				= Vec2(32, 34);
		
		Is_Icon_Direction_Based	= true;
		Is_Icon_Overlay			= true;
		
		Combine_Name			= "Ball Cap";
		Combine_Group			= "Hats";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
	}
}
