// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_uzi;

[
	Placeable(true), 
	Name("Uzi Pickup"), 
	Description("Weapon Pickup") 
]
public class Uzi_Pickup : Weapon_Pickup
{
	Uzi_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_uzi_0";
		Pickup_Name		= Locale.Get("#item_uzi_name");
		Pickup_Sound	= "sfx_objects_pickups_uzi_pickup";
		Weapon_Type		= typeof(Weapon_Uzi);

		Cost			= 500;
	}
}