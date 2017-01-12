// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_love_cannon;

[
	Placeable(true), 
	Name("Love Cannon Pickup"), 
	Description("Weapon Pickup") 
]
public class Love_Cannon_Pickup : Weapon_Pickup
{
	Love_Cannon_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_lovecannon_0";
		Pickup_Name		= Locale.Get("#item_love_cannon_name");
		Pickup_Sound	= "sfx_objects_pickups_lovecannon_pickup";
		Weapon_Type		= typeof(Weapon_Love_Cannon);

		Cost			= 3000;
	}
}