// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.ammo.ammo;
using game.weapons.ammo.weapon_ammo_freeze;

[
	Placeable(true), 
	Name("Freeze Ammo Pickup"), 
	Description("Weapon Pickup") 
]
public class Freeze_Ammo_Pickup : Ammo_Pickup
{
	Freeze_Ammo_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_freezeammo_0";
		Pickup_Name		= Locale.Get("#item_freeze_ammo_name");
		Pickup_Sound	= "sfx_objects_pickups_freezeammo_pickup";
		Ammo_Type		= typeof(Weapon_Ammo_Freeze);

		Cost			= 1000;
	}
}