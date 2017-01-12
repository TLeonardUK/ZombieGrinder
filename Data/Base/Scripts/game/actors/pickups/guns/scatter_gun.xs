// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.shotgun.weapon_scatter_gun;

[
	Placeable(true), 
	Name("Scatter Gun Pickup"), 
	Description("Weapon Pickup") 
]
public class Scatter_Gun_Pickup : Weapon_Pickup
{
	Scatter_Gun_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_scatter_gun_0";
		Pickup_Name		= Locale.Get("#item_scattergun_name");
		Pickup_Sound	= "sfx_objects_pickups_scattergun_pickup";
		Weapon_Type		= typeof(Weapon_Scatter_Gun);

		Cost			= 1500;
	}
}