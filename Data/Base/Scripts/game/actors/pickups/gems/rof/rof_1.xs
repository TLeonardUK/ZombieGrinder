// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_yellow_gem_1;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_ROF_1_Pickup : Local_Item_Pickup
{
	Gem_ROF_1_Pickup()
	{
		Pickup_Sprite	= "item_gem_rof_1_0";
		Pickup_Name		= Locale.Get("#item_yellow_gem_1_name");
		Archetype		= typeof(Item_Yellow_Gem_1);

		Cost			= 9999999;
	}
}