// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Large Armor Pickup"), 
	Description("AAAAAAAAAARRRRRRRRRMOOOOOOR.") 
]
public class Large_Armor_Pickup : Health_Pickup
{
	Large_Armor_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_armour_large_0";
		Pickup_Name		= Locale.Get("#item_armor_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 0;
		Armor_Points	= 100;

		Cost			= 500;
	}
}