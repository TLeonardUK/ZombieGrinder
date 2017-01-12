// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_freeze_ray;

[
	Placeable(true), 
	Name("Freeze Ray Pickup"), 
	Description("Weapon Pickup") 
]
public class Freeze_Ray_Pickup : Weapon_Pickup
{
	Freeze_Ray_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_freeze_ray_0";
		Pickup_Name		= Locale.Get("#item_freeze_ray_name");
		Pickup_Sound	= "sfx_objects_pickups_freezeray_pickup";
		Weapon_Type		= typeof(Weapon_Freeze_Ray);

		Cost			= 1500;
	}
}