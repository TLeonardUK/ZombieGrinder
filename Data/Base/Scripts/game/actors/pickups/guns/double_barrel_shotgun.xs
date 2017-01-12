// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_db_shotgun;

[
	Placeable(true), 
	Name("DB Shotgun Pickup"), 
	Description("Weapon Pickup") 
]
public class DB_Shotgun_Pickup : Weapon_Pickup
{
	DB_Shotgun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_doublebarrelshotgun_0";
		Pickup_Name		= Locale.Get("#item_double_barrel_shotgun_name");
		Pickup_Sound	= "sfx_objects_pickups_doublebarrelshotgun_pickup";
		Weapon_Type		= typeof(Weapon_DB_Shotgun);

		Cost			= 1000;
	}
}