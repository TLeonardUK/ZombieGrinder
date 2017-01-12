// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_aqua_gem_5;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_XP_5_Pickup : Local_Item_Pickup
{
	Gem_XP_5_Pickup()
	{
		Pickup_Sprite	= "item_gem_xp_5_0";
		Pickup_Name		= Locale.Get("#item_aqua_gem_5_name");
		Archetype		= typeof(Item_Aqua_Gem_5);

		Cost			= 9999999;
	}
}