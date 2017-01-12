// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_shotgun;

[
	Placeable(true), 
	Name("Shotgun Pickup"), 
	Description("Weapon Pickup") 
]
public class Shotgun_Pickup : Weapon_Pickup
{
	Shotgun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_shotgun_0";
		Pickup_Name		= Locale.Get("#item_shotgun_name");
		Pickup_Sound	= "sfx_objects_pickups_shotgun_pickup";
		Weapon_Type		= typeof(Weapon_Shotgun);

		Cost			= 500;
	}
}