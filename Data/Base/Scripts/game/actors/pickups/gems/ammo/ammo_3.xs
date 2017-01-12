// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.pickups.item_pickup;
using game.items.gems.item_purple_gem_3;

[
	Placeable(false), 
	Name("Gem Pickup"), 
	Description("A gem.") 
]
public class Gem_Ammo_3_Pickup : Local_Item_Pickup
{
	Gem_Ammo_3_Pickup()
	{
		Pickup_Sprite	= "item_gem_ammocapacity_3_0";
		Pickup_Name		= Locale.Get("#item_purple_gem_3_name");
		Archetype		= typeof(Item_Purple_Gem_3);

		Cost			= 9999999;
	}
}