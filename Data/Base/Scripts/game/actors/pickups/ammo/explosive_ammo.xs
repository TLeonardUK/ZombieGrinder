// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.ammo.ammo;
using game.weapons.ammo.weapon_ammo_explosive;

[
	Placeable(true), 
	Name("Explosive Ammo Pickup"), 
	Description("Weapon Pickup") 
]
public class Explosive_Ammo_Pickup : Ammo_Pickup
{
	Explosive_Ammo_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_explosiveammo_0";
		Pickup_Name		= Locale.Get("#item_explosive_ammo_name");
		Pickup_Sound	= "sfx_objects_pickups_explosiveammo_pickup";
		Ammo_Type		= typeof(Weapon_Ammo_Explosive);

		Cost			= 1000;
	}
}