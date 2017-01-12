// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_red_gem_5;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_Health_5_Pickup : Local_Item_Pickup
{
	Gem_Health_5_Pickup()
	{
		Pickup_Sprite	= "item_gem_health_5_0";
		Pickup_Name		= Locale.Get("#item_red_gem_5_name");
		Archetype		= typeof(Item_Red_Gem_5);

		Cost			= 9999999;
	}
}