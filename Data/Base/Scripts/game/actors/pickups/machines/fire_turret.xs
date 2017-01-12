// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.machines.turret;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Placeable(true), 
	Name("Fire Turret Pickup"), 
	Description("AAAAAAAAAARRRRRRRRRMOOOOOOR.") 
]
public class Fire_Turret_Pickup : Turret_Pickup
{
	Fire_Turret_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_fire_turret_0";
		Pickup_Name		= Locale.Get("#item_fire_turret_name");
		Ammo_Type		= typeof(Weapon_Ammo_Incendiary);

		Cost			= 2000;
	}
}