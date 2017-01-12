// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Small Armor Pickup"), 
	Description("AAAAAAAAAARRRRRRRRRMOOOOOOR.") 
]
public class Small_Armor_Pickup : Health_Pickup
{
	Small_Armor_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_armour_small_0";
		Pickup_Name		= Locale.Get("#item_armor_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 0;
		Armor_Points	= 25;

		Cost			= 50;
	}
}