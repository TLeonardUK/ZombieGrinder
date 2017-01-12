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
public class Medium_Gem_Grab_Bag_Drop_Table : Drop_Table
{
	Medium_Gem_Grab_Bag_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
		Add(typeof(Item_Aqua_Gem_2),		5);
		Add(typeof(Item_Blue_Gem_2),		5);
		Add(typeof(Item_Green_Gem_2),		5);
		Add(typeof(Item_Orange_Gem_2),		5);
		Add(typeof(Item_Pink_Gem_2),		5);
		Add(typeof(Item_Purple_Gem_2),		5);
		Add(typeof(Item_Red_Gem_2),			5);
		Add(typeof(Item_Yellow_Gem_2),		5);

		Add(typeof(Item_Aqua_Gem_3),		2);
		Add(typeof(Item_Blue_Gem_3),		2);
		Add(typeof(Item_Green_Gem_3),		2);
		Add(typeof(Item_Orange_Gem_3),		2);
		Add(typeof(Item_Pink_Gem_3),		2);
		Add(typeof(Item_Purple_Gem_3),		2);
		Add(typeof(Item_Red_Gem_3),			2);
		Add(typeof(Item_Yellow_Gem_3),		2);		
	}
}