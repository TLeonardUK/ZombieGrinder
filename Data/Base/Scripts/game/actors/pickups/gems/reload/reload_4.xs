// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_orange_gem_4;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_Reload_4_Pickup : Local_Item_Pickup
{
	Gem_Reload_4_Pickup()
	{
		Pickup_Sprite	= "item_gem_reload_4_0";
		Pickup_Name		= Locale.Get("#item_orange_gem_4_name");
		Archetype		= typeof(Item_Orange_Gem_4);

		Cost			= 9999999;
	}
}