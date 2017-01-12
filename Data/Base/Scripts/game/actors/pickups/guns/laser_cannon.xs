// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_laser_cannon;

[
	Placeable(true), 
	Name("Laser Cannon Pickup"), 
	Description("Weapon Pickup") 
]
public class Laser_Cannon_Pickup : Weapon_Pickup
{
	Laser_Cannon_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_lasercannon_0";
		Pickup_Name		= Locale.Get("#item_laser_cannon_name");
		Pickup_Sound	= "sfx_objects_pickups_lasercannon_pickup";
		Weapon_Type		= typeof(Weapon_Laser_Cannon);

		Cost			= 5000;
	}
}