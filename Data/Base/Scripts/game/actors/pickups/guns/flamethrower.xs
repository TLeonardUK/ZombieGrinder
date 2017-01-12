// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_flamethrower;

[
	Placeable(true), 
	Name("Flamethrower Pickup"), 
	Description("Weapon Pickup") 
]
public class Flamethrower_Pickup : Weapon_Pickup
{
	Flamethrower_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_flamethrower_0";
		Pickup_Name		= Locale.Get("#item_flamethrower_name");
		Pickup_Sound	= "sfx_objects_pickups_flamethrower_pickup";
		Weapon_Type		= typeof(Weapon_Flamethrower);

		Cost			= 2000;
	}
}