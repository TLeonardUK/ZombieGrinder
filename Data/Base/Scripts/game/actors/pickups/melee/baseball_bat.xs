// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.melee.weapon_baseball_bat;

[
	Placeable(true), 
	Name("Baseball Bat Pickup"), 
	Description("Weapon Pickup") 
]
public class Baseball_Bat_Pickup : Weapon_Pickup
{
	Baseball_Bat_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_baseball_bat_0";
		Pickup_Name		= Locale.Get("#item_baseball_bat_name");
		Pickup_Sound	= "sfx_objects_pickups_melee_pickup";
		Weapon_Type		= typeof(Weapon_Baseball_Bat);

		Cost			= 500;
	}
}