// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.machines.turret;
using game.weapons.ammo.weapon_ammo_explosive;

[
	Placeable(true), 
	Name("Rocket Turret Pickup"), 
	Description("AAAAAAAAAARRRRRRRRRMOOOOOOR.") 
]
public class Rocket_Turret_Pickup : Turret_Pickup
{
	Rocket_Turret_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_rocket_turret_0";
		Pickup_Name		= Locale.Get("#item_rocket_turret_name");
		Ammo_Type		= typeof(Weapon_Ammo_Explosive);

		Cost			= 2000;
	}
}