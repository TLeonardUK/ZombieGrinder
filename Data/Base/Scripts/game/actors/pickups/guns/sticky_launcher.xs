// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_sticky_launcher;

[
	Placeable(true), 
	Name("Sticky Launcher Pickup"), 
	Description("Weapon Pickup") 
]
public class Sticky_Launcher_Pickup : Weapon_Pickup
{
	Sticky_Launcher_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_sticky_launcher_0";
		Pickup_Name		= Locale.Get("#item_sticky_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_sticky_launcher_pickup";
		Weapon_Type		= typeof(Weapon_Sticky_Launcher);

		Cost			= 1500;
	}
}