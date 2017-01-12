// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_spaz;

[
	Placeable(true), 
	Name("Spaz Pickup"), 
	Description("Weapon Pickup") 
]
public class Spaz_Pickup : Weapon_Pickup
{
	Spaz_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_spaz_0";
		Pickup_Name		= Locale.Get("#item_spaz_name");
		Pickup_Sound	= "sfx_objects_pickups_spaz_pickup";
		Weapon_Type		= typeof(Weapon_Spaz);

		Cost			= 2500;
	}
}