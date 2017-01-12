// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.weapon_pickup;
using game.weapons.heavy.weapon_rpc;

[
	Placeable(true), 
	Name("RPC Pickup"), 
	Description("Weapon Pickup") 
]
public class RPC_Pickup : Weapon_Pickup
{
	RPC_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_rpc_0";
		Pickup_Name		= Locale.Get("#item_rpc_name");
		Pickup_Sound	= "sfx_objects_pickups_rpc_pickup";
		Weapon_Type		= typeof(Weapon_RPC);

		Cost			= 3000;
	}
}