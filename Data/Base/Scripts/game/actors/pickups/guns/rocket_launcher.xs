// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_rocket_launcher;

[
	Placeable(true), 
	Name("Rocket Launcher Pickup"), 
	Description("Weapon Pickup") 
]
public class Rocket_Launcher_Pickup : Weapon_Pickup
{
	Rocket_Launcher_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_rocket_launcher_0";
		Pickup_Name		= Locale.Get("#item_rocket_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_rocket_launcher_pickup";
		Weapon_Type		= typeof(Weapon_Rocket_Launcher);

		Cost			= 3000;
	}
}