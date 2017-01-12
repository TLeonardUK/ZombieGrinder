// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_spaz;

[
	Placeable(true), 
	Name("Barricade Pickup"), 
	Description("Weapon Pickup") 
]
public class Barricade_Pickup : Weapon_Pickup
{
	Barricade_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_Barricade_0";
		Pickup_Name		= Locale.Get("#item_barricade_name");
		Pickup_Sound	= "sfx_objects_pickups_barricade_pickup";
		Weapon_Type		= typeof(Weapon_Barricade);

		Cost			= 2500;
	}
}