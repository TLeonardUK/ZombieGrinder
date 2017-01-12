// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_red_gem_2;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_Health_2_Pickup : Local_Item_Pickup
{
	Gem_Health_2_Pickup()
	{
		Pickup_Sprite	= "item_gem_health_2_0";
		Pickup_Name		= Locale.Get("#item_red_gem_2_name");
		Archetype		= typeof(Item_Red_Gem_2);

		Cost			= 9999999;
	}
}