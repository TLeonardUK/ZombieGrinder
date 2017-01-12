// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_green_gem_2;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_Damage_2_Pickup : Local_Item_Pickup
{
	Gem_Damage_2_Pickup()
	{
		Pickup_Sprite	= "item_gem_damage_2_0";
		Pickup_Name		= Locale.Get("#item_green_gem_2_name");
		Archetype		= typeof(Item_Green_Gem_2);

		Cost			= 9999999;
	}
}