// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.rifle.weapon_shock_rifle;

[
	Placeable(true), 
	Name("Shock Rifle Pickup"), 
	Description("Weapon Pickup") 
]
public class Shock_Rifle_Pickup : Weapon_Pickup
{
	Shock_Rifle_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_shock_rifle_0";
		Pickup_Name		= Locale.Get("#item_shock_rifle_name");
		Pickup_Sound	= "sfx_objects_pickups_shockrifle_pickup";
		Weapon_Type		= typeof(Weapon_Shock_Rifle);

		Cost			= 2000;
	}
}