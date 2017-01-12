// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_grenade_launcher;

[
	Placeable(true), 
	Name("Grenade Launcher Pickup"), 
	Description("Weapon Pickup") 
]
public class Grenade_Launcher_Pickup : Weapon_Pickup
{
	Grenade_Launcher_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_grenade_launcher_0";
		Pickup_Name		= Locale.Get("#item_grenade_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_grenade_launcher_pickup";
		Weapon_Type		= typeof(Weapon_Grenade_Launcher);

		Cost			= 1500;
	}
}