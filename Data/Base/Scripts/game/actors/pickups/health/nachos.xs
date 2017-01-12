// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Nachos Pickup"), 
	Description("Heaaaaaaaaaaaaaaalth") 
]
public class Nachos_Pickup : Health_Pickup
{
	Nachos_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_nachos_0";
		Pickup_Name		= Locale.Get("#item_medipack_name");
		Pickup_Sound	= "sfx_objects_pickups_eat";

		Health_Points	= 50;
		Armor_Points	= 0;

		Cost			= 250;
	}
}