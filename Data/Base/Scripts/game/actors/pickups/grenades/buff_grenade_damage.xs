// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_buff_grenade_damage;

[
	Placeable(true), 
	Name("Buff Grenade (Damage) Pickup"), 
	Description("Weapon Pickup") 
]
public class Buff_Grenade_Damage_Pickup : Weapon_Pickup
{
	Buff_Grenade_Damage_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_buff_grenade_damage_0";
		Pickup_Name		= Locale.Get("#item_buff_grenade_damage_name");
		Pickup_Sound	= "sfx_objects_pickups_buffgrenade_pickup";
		Weapon_Type		= typeof(Weapon_Buff_Grenade_Damage);

		Cost			= 2000;
	}
}