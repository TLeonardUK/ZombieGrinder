// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.drops.drop_table;
using game.actors.pickups.pickup_list;

[
	Placeable(false), 
	Name("Challenge Drop Table"), 
	Description("Drop table for challenge items.") 
]
public class Challenge_XP_Drop_Table : Drop_Table
{
	Challenge_XP_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
        // Coin bags!
		Add(typeof(Item_Small_XP_Grab_Bag), 10);
		Add(typeof(Item_Medium_XP_Grab_Bag), 3);
		Add(typeof(Item_Large_XP_Grab_Bag), 1);
	}
}