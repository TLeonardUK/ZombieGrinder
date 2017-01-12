// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.melee.weapon_sword;

[
	Placeable(true), 
	Name("Sword Pickup"), 
	Description("Weapon Pickup") 
]
public class Sword_Pickup : Weapon_Pickup
{
	Sword_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_sword_0";
		Pickup_Name		= Locale.Get("#item_sword_name");
		Pickup_Sound	= "sfx_objects_pickups_melee_pickup";
		Weapon_Type		= typeof(Weapon_Sword);

		Cost			= 2000;
	}
}