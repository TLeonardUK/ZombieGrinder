// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.melee.weapon_fight_saber;

[
	Placeable(true), 
	Name("Fight Saber Pickup"), 
	Description("Weapon Pickup") 
]
public class Fight_Saber_Pickup : Weapon_Pickup
{
	Fight_Saber_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_fight_saber_0";
		Pickup_Name		= Locale.Get("#item_fight_saber_name");
		Pickup_Sound	= "sfx_objects_pickups_melee_pickup";
		Weapon_Type		= typeof(Weapon_Fight_Saber);

		Cost			= 2000;
	}
}