// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_molotov;

[
	Placeable(true), 
	Name("Molotov Pickup"), 
	Description("Weapon Pickup") 
]
public class Molotov_Pickup : Weapon_Pickup
{
	Molotov_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_molotov_0";
		Pickup_Name		= Locale.Get("#item_molotov_name");
		Pickup_Sound	= "sfx_objects_pickups_molotov_pickup";
		Weapon_Type		= typeof(Weapon_Molotov);

		Cost			= 2000;
	}
}