// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.inventory_pickup;

[
	Placeable(true), 
	Name("Key Pickup"), 
	Description("Gold, Shiney") 
]
public class Key_Pickup : Inventory_Pickup
{
	Key_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_key_0";
		Pickup_Name		= "<internal-name>";
		Pickup_Sound	= "sfx_objects_pickups_key_pickup";
		Do_Not_Dispose_On_Despawn	= true;
		Can_Respawn_On_Screen = true;
	}
}