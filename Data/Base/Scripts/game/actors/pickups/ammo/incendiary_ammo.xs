// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.ammo.ammo;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Placeable(true), 
	Name("Incendiary Ammo Pickup"), 
	Description("Weapon Pickup") 
]
public class Incendiary_Ammo_Pickup : Ammo_Pickup
{
	Incendiary_Ammo_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_incendiaryammo_0";
		Pickup_Name		= Locale.Get("#item_incendiary_ammo_name");
		Pickup_Sound	= "sfx_objects_pickups_incendiaryammo_pickup";
		Ammo_Type		= typeof(Weapon_Ammo_Incendiary);

		Cost			= 1000;
	}
}