// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.drops.drop_table;
using game.actors.pickups.pickup_list;

[
	Placeable(false), 
	Name("Gem Drop Table"), 
	Description("Drop table for gems.") 
]
public class Gem_Drop_Table : Drop_Table
{
	Gem_Drop_Table()
	{
		Drop_Probability = 0.04f; // 4%
		
		Add(typeof(Gem_Ammo_1_Pickup),		100);
		Add(typeof(Gem_Ammo_2_Pickup),		10);
	//	Add(typeof(Gem_Ammo_3_Pickup),		25);
	//	Add(typeof(Gem_Ammo_4_Pickup),		12);
	//	Add(typeof(Gem_Ammo_5_Pickup),		6);

		Add(typeof(Gem_Damage_1_Pickup),	100);
		Add(typeof(Gem_Damage_2_Pickup),	10);
	//	Add(typeof(Gem_Damage_3_Pickup),	25);
	//	Add(typeof(Gem_Damage_4_Pickup),	12);
	//	Add(typeof(Gem_Damage_5_Pickup),	6);

		Add(typeof(Gem_Health_1_Pickup),	100);
		Add(typeof(Gem_Health_2_Pickup),	10);
	//	Add(typeof(Gem_Health_3_Pickup),	25);
	//	Add(typeof(Gem_Health_4_Pickup),	12);
	//	Add(typeof(Gem_Health_5_Pickup),	6);
		
		Add(typeof(Gem_XP_1_Pickup),		100);
		Add(typeof(Gem_XP_2_Pickup),		10);
	//	Add(typeof(Gem_XP_3_Pickup),		25);
	//	Add(typeof(Gem_XP_4_Pickup),		12);
	//	Add(typeof(Gem_XP_5_Pickup),		6);

		Add(typeof(Gem_Speed_1_Pickup),		100);
		Add(typeof(Gem_Speed_2_Pickup),		10);
	//	Add(typeof(Gem_Speed_3_Pickup),		25);
	//	Add(typeof(Gem_Speed_4_Pickup),		12);
	//	Add(typeof(Gem_Speed_5_Pickup),		6);

		Add(typeof(Gem_ROF_1_Pickup),		100);
		Add(typeof(Gem_ROF_2_Pickup),		10);
	//	Add(typeof(Gem_ROF_3_Pickup),		25);
	//	Add(typeof(Gem_ROF_4_Pickup),		12);
	//	Add(typeof(Gem_ROF_5_Pickup),		6);

		Add(typeof(Gem_Reload_1_Pickup),	100);
		Add(typeof(Gem_Reload_2_Pickup),	10);
	//	Add(typeof(Gem_Reload_3_Pickup),	25);
	//	Add(typeof(Gem_Reload_4_Pickup),	12);
	//	Add(typeof(Gem_Reload_5_Pickup),	6);

		Add(typeof(Gem_Price_1_Pickup),		100);
		Add(typeof(Gem_Price_2_Pickup),		10);
	//	Add(typeof(Gem_Price_3_Pickup),		25);
	//	Add(typeof(Gem_Price_4_Pickup),		12);
	//	Add(typeof(Gem_Price_5_Pickup),		6);
	}
}