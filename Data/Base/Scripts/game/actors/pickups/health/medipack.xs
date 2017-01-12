// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Medipack Pickup"), 
	Description("Heaaaaaaaaaaaaaaalth") 
]
public class Medipack_Pickup : Health_Pickup
{
	Medipack_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_medipack_0";
		Pickup_Name		= Locale.Get("#item_medipack_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 100;
		Armor_Points	= 0;

		Cost			= 500;
	}
}