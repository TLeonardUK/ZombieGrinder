// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_tripmine;

[
	Placeable(true), 
	Name("Tripmine Pickup"), 
	Description("Weapon Pickup") 
]
public class Tripmine_Pickup : Weapon_Pickup
{
	Tripmine_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_tripmine_0";
		Pickup_Name		= Locale.Get("#item_tripmine_name");
		Pickup_Sound	= "sfx_objects_pickups_tripmine_pickup";
		Weapon_Type		= typeof(Weapon_Tripmine);

		Cost			= 1000;
	}
}