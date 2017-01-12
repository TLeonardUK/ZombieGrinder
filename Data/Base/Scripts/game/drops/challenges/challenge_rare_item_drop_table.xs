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
public class Challenge_Rare_Item_Drop_Table : Drop_Table
{
	Challenge_Rare_Item_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
        // Gems Grab Bags!
		Add(typeof(Item_Medium_Gem_Grab_Bag), 10);
		Add(typeof(Item_Large_Gem_Grab_Bag), 10);
		Add(typeof(Item_Spanner), 			  3);
        
        // Individual Top-Tier Gems!
		Add(typeof(Item_Aqua_Gem_5),		5);
		Add(typeof(Item_Blue_Gem_5),		5);
		Add(typeof(Item_Green_Gem_5),		5);
		Add(typeof(Item_Orange_Gem_5),		5);
		Add(typeof(Item_Pink_Gem_5),		5);
		Add(typeof(Item_Purple_Gem_5),		5);
		Add(typeof(Item_Red_Gem_5),			5);
		Add(typeof(Item_Yellow_Gem_5),		5);

		// Hat.
		Add(typeof(Item_Cheer_Bringer),		10);
		Add(typeof(Item_Presento),	   	    10);
	}
}