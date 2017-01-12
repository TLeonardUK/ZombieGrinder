// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.inventory_pickup;

[
	Placeable(true), 
	Name("Bag Pickup"), 
	Description("Pickup my bag brah") 
]
public class Bag_Pickup : Inventory_Pickup
{
	Bag_Pickup()
	{
		Pickup_Sprite	= "actor_pickups_bag_0";
		Pickup_Name		= "<internal-name>";
		Pickup_Sound	= "sfx_objects_pickups_bag_pickup";
		Do_Not_Dispose_On_Despawn	= true;
		Can_Respawn_On_Screen = true;
	}
}