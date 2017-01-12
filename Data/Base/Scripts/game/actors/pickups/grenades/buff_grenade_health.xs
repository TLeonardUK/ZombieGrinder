// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_buff_grenade_health;

[
	Placeable(true), 
	Name("Buff Grenade (Health) Pickup"), 
	Description("Weapon Pickup") 
]
public class Buff_Grenade_Health_Pickup : Weapon_Pickup
{
	Buff_Grenade_Health_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_buff_grenade_health_0";
		Pickup_Name		= Locale.Get("#item_buff_grenade_health_name");
		Pickup_Sound	= "sfx_objects_pickups_buffgrenade_pickup";
		Weapon_Type		= typeof(Weapon_Buff_Grenade_Health);

		Cost			= 2000;
	}
}