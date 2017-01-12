// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.pistol.weapon_flare_gun;

[
	Placeable(true), 
	Name("Flaregun Pickup"), 
	Description("Weapon Pickup") 
]
public class Flaregun_Pickup : Weapon_Pickup
{
	Flaregun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_flaregun_0";
		Pickup_Name		= Locale.Get("#item_flare_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_flaregun_pickup";
		Weapon_Type		= typeof(Weapon_Flare_Gun);

		Cost			= 1000;
	}
}