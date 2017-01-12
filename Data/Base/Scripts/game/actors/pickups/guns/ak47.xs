// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.rifle.weapon_ak47;

[
	Placeable(true), 
	Name("AK47 Pickup"), 
	Description("Weapon Pickup") 
]
public class AK47_Pickup : Weapon_Pickup
{
	AK47_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_ak47_0";
		Pickup_Name		= Locale.Get("#item_ak47_name");
		Pickup_Sound	= "sfx_objects_pickups_ak47_pickup";
		Weapon_Type		= typeof(Weapon_Ak47);

		Cost			= 1000;
	}
}