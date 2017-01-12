// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_spaz;

[
	Placeable(true), 
	Name("Pistol Pickup"), 
	Description("Weapon Pickup") 
]
public class Pistol_Pickup : Weapon_Pickup
{
	Pistol_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_pistol_0";
		Pickup_Name		= Locale.Get("#item_pistol_name");
		Pickup_Sound	= "sfx_objects_pickups_pistol_pickup";
		Weapon_Type		= typeof(Weapon_Pistol);

		Cost			= 2500;
	}
}