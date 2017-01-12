// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_buff_grenade_rof;

[
	Placeable(true), 
	Name("Buff Grenade (ROF) Pickup"), 
	Description("Weapon Pickup") 
]
public class Buff_Grenade_ROF_Pickup : Weapon_Pickup
{
	Buff_Grenade_ROF_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_buff_grenade_rof_0";
		Pickup_Name		= Locale.Get("#item_buff_grenade_rof_name");
		Pickup_Sound	= "sfx_objects_pickups_buffgrenade_pickup";
		Weapon_Type		= typeof(Weapon_Buff_Grenade_ROF);

		Cost			= 2000;
	}
}