// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using weapon_example_gun;

public class Item_Example_Gun : Item_Archetype
{
	Item_Example_Gun()
	{
		Name 					= "Example Gun";
		Description 			= "An example of adding a custom weapon!";
		Category				= "Items";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 1000;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= true;
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_example_weapon";
		Icon_Offset				= Vec2(16, 10);
				
		Body_Animation			= "avatar_body_example_gun";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Example_Gun);
		
		Combine_Name			= "Example Gun";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Gems", Item_Combine_Method.Attach, "Nothing");
	}
}
