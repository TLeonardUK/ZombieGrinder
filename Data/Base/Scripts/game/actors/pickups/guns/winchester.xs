// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.rifle.weapon_winchester_rifle;

[
	Placeable(true), 
	Name("Winchester Rifle Pickup"), 
	Description("Weapon Pickup") 
]
public class Winchester_Rifle_Pickup : Weapon_Pickup
{
	Winchester_Rifle_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_winchester_rifle_0";
		Pickup_Name		= Locale.Get("#item_winchester_rifle_name");
		Pickup_Sound	= "sfx_objects_pickups_winchester_rifle_pickup";
		Weapon_Type		= typeof(Weapon_Winchester_Rifle);

		Cost			= 2500;
	}
}