// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_acid_gun;

[
	Placeable(true), 
	Name("Acid Gun Pickup"), 
	Description("Weapon Pickup") 
]
public class Acid_Gun_Pickup : Weapon_Pickup
{
	Acid_Gun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_acid_gun_0";
		Pickup_Name		= Locale.Get("#item_acid_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_acid_gun_pickup";
		Weapon_Type		= typeof(Weapon_Acid_Gun);

		Cost			= 2000;
	}
}