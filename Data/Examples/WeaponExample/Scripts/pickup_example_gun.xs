// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using weapon_example_gun;

[
	Placeable(true), 
	Name("Boltgun Pickup"), 
	Description("Weapon Pickup") 
]
public class Example_Gun_Pickup : Weapon_Pickup
{
	Example_Gun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_example_gun_0";
		Pickup_Name		= "Example Gun";
		Pickup_Sound	= "sfx_example_gun_pickup";
		Weapon_Type		= typeof(Weapon_Example_Gun);

		Cost			= 1500;
	}
}