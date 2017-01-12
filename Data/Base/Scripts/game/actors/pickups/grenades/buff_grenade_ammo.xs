// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_buff_grenade_ammo;

[
	Placeable(true), 
	Name("Buff Grenade (Ammo) Pickup"), 
	Description("Weapon Pickup") 
]
public class Buff_Grenade_Ammo_Pickup : Weapon_Pickup
{
	Buff_Grenade_Ammo_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_buff_grenade_ammo_0";
		Pickup_Name		= Locale.Get("#item_buff_grenade_ammo_name");
		Pickup_Sound	= "sfx_objects_pickups_buffgrenade_pickup";
		Weapon_Type		= typeof(Weapon_Buff_Grenade_Ammo);

		Cost			= 2000;
	}
}