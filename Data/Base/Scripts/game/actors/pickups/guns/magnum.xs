// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.pistol.weapon_magnum;

[
	Placeable(true), 
	Name("Magnum Pickup"), 
	Description("Weapon Pickup") 
]
public class Magnum_Pickup : Weapon_Pickup
{
	Magnum_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_magnum_0";
		Pickup_Name		= Locale.Get("#item_magnum_name");
		Pickup_Sound	= "sfx_objects_pickups_magnum_pickup";
		Weapon_Type		= typeof(Weapon_Magnum);

		Cost			= 1500;
	}
}