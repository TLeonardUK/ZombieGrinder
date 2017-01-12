// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Pill Pickup"), 
	Description("Heaaaaaaaaaaaaaaalth") 
]
public class Pill_Pickup : Health_Pickup
{
	Pill_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_pill_0";
		Pickup_Name		= Locale.Get("#item_pill_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 10;
		Armor_Points	= 0;

		Cost			= 50;
	}
}