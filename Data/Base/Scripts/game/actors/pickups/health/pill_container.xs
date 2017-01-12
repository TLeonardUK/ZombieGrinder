// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.health_pickup;

[
	Placeable(true), 
	Name("Pill Container Pickup"), 
	Description("Heaaaaaaaaaaaaaaalth") 
]
public class Pill_Container_Pickup : Health_Pickup
{
	Pill_Container_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_pillcontainer_0";
		Pickup_Name		= Locale.Get("#item_pill_container_name");
		Pickup_Sound	= "sfx_objects_pickups_swallow";

		Health_Points	= 25;
		Armor_Points	= 0;

		Cost			= 125;
	}
}