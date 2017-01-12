// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_bolt_gun;

[
	Placeable(true), 
	Name("Boltgun Pickup"), 
	Description("Weapon Pickup") 
]
public class Boltgun_Pickup : Weapon_Pickup
{
	Boltgun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_bolt_gun_0";
		Pickup_Name		= Locale.Get("#item_bolt_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_boltgun_pickup";
		Weapon_Type		= typeof(Weapon_Bolt_Gun);

		Cost			= 1500;
	}
}