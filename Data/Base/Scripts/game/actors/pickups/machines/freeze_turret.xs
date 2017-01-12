// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.machines.turret;
using game.weapons.ammo.weapon_ammo_freeze;

[
	Placeable(true), 
	Name("Freeze Turret Pickup"), 
	Description("AAAAAAAAAARRRRRRRRRMOOOOOOR.") 
]
public class Freeze_Turret_Pickup : Turret_Pickup
{
	Freeze_Turret_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_freeze_turret_0";
		Pickup_Name		= Locale.Get("#item_freeze_turret_name");
		Ammo_Type		= typeof(Weapon_Ammo_Freeze);

		Cost			= 2000;
	}
}