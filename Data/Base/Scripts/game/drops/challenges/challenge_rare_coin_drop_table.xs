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
public class Challenge_Rare_Coin_Drop_Table : Drop_Table
{
	Challenge_Rare_Coin_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
        // Coin bags!
		Add(typeof(Item_Medium_Coin_Grab_Bag), 5);
		Add(typeof(Item_Large_Coin_Grab_Bag), 5);
	}
}