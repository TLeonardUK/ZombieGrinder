// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.grenade.weapon_frag;

[
	Placeable(true), 
	Name("Frag Pickup"), 
	Description("Weapon Pickup") 
]
public class Frag_Pickup : Weapon_Pickup
{
	Frag_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_frag_0";
		Pickup_Name		= Locale.Get("#item_frag_name");
		Pickup_Sound	= "sfx_objects_pickups_frag_pickup";
		Weapon_Type		= typeof(Weapon_Frag);

		Cost			= 2000;
	}
}