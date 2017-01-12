// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_love_thrower;

[
	Placeable(true), 
	Name("Love Thrower Pickup"), 
	Description("Weapon Pickup") 
]
public class Love_Thrower_Pickup : Weapon_Pickup
{
	Love_Thrower_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_lovethrower_0";
		Pickup_Name		= Locale.Get("#item_love_thrower_name");
		Pickup_Sound	= "sfx_objects_pickups_lovethrower_pickup";
		Weapon_Type		= typeof(Weapon_Love_Thrower);

		Cost			= 3000;
	}
}