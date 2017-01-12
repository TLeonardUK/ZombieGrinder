// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Drink Pickup"), 
	Description("Glug glug glug.") 
]
public class Drink_Pickup : Health_Pickup
{
	Drink_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_drink_0";
		Pickup_Name		= Locale.Get("#item_drink_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 50;
		Armor_Points	= 0;

		Cost			= 250;
	}
}