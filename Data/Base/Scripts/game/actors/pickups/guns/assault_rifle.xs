// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.rifle.weapon_assault_rifle;

[
	Placeable(true), 
	Name("Assault Rifle Pickup"), 
	Description("Weapon Pickup") 
]
public class Assault_Rifle_Pickup : Weapon_Pickup
{
	Assault_Rifle_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_m16_0";
		Pickup_Name		= Locale.Get("#item_assault_rifle_name");
		Pickup_Sound	= "sfx_objects_pickups_m16_pickup";
		Weapon_Type		= typeof(Weapon_Assault_Rifle);

		Cost			= 2000;
	}
}