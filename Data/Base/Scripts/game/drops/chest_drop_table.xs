// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.drops.drop_table;
using game.actors.pickups.pickup_list;

[
	Placeable(false), 
	Name("Chest Drop Table"), 
	Description("Drop table for chest items.") 
]
public class Chest_Drop_Table : Drop_Table
{
	Chest_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
		Add(typeof(Item_Spanner), 			 5);
		
		// Top tier gems.
		Add(typeof(Item_Aqua_Gem_4),		12);
		Add(typeof(Item_Aqua_Gem_5),		6);
		Add(typeof(Item_Blue_Gem_4),		12);
		Add(typeof(Item_Blue_Gem_5),		6);
		Add(typeof(Item_Green_Gem_4),		12);
		Add(typeof(Item_Green_Gem_5),		6);
		Add(typeof(Item_Orange_Gem_4),		12);
		Add(typeof(Item_Orange_Gem_5),		6);
		Add(typeof(Item_Pink_Gem_4),		12);
		Add(typeof(Item_Pink_Gem_5),		6);
		Add(typeof(Item_Purple_Gem_4),		12);
		Add(typeof(Item_Purple_Gem_5),		6);
		Add(typeof(Item_Red_Gem_4),			12);
		Add(typeof(Item_Red_Gem_5),			6);
		Add(typeof(Item_Yellow_Gem_4),		12);
		Add(typeof(Item_Yellow_Gem_5),		6);

		// Hats! Glorious hats! Hat simulator 2000!
		Add(typeof(Item_Cheer_Bringer),		2);
		Add(typeof(Item_Presento),	   	    2);

		Add(typeof(Item_Antlers),			1);
		Add(typeof(Item_Armour),			1);
		Add(typeof(Item_Aviators),			1);
		Add(typeof(Item_Bow),				1);
		Add(typeof(Item_Ghetto_Superstar),	1);
		Add(typeof(Item_Glasses),			1);
		Add(typeof(Item_Headset),			1);
		Add(typeof(Item_Scouter),			1);
		Add(typeof(Item_Ami),				1);
		Add(typeof(Item_Shades),			1);
		Add(typeof(Item_Ball_Cap),			1);
		Add(typeof(Item_Beanie_1),			1);
		Add(typeof(Item_Beanie_2),			1);
		Add(typeof(Item_Dantz),				1);
		Add(typeof(Item_Desgardes),			1);
		Add(typeof(Item_Doppleganger),		1);
		Add(typeof(Item_Flat_Top),			1);
		Add(typeof(Item_Fred_Head),			1);
		Add(typeof(Item_Gatsby),			1);
		Add(typeof(Item_Gi_Joe),			1);
		Add(typeof(Item_Golfers_Delight),	1);
		Add(typeof(Item_Little_Sis),		1);
		Add(typeof(Item_Louisiana_Jones),	1);
		Add(typeof(Item_Mami),				1);
		Add(typeof(Item_Nekomimi),			1);
		Add(typeof(Item_Pigtails),			1);
		Add(typeof(Item_Player1hair),		1);
		Add(typeof(Item_Player2hair),		1);
		Add(typeof(Item_Player3hair),		1);
		Add(typeof(Item_Player4hair),		1);
		Add(typeof(Item_Pompador),			1);
		Add(typeof(Item_Poniteru),			1);
		Add(typeof(Item_Pumpkin),			1);
		Add(typeof(Item_Scruffy_Sidekick),	1);
		Add(typeof(Item_Shizue),			1);
		Add(typeof(Item_Skull),				1);
		Add(typeof(Item_Thugpomp),			1);
		Add(typeof(Item_Top_Hat),			16); // world needs more class
		Add(typeof(Item_Trucker),			1);
		Add(typeof(Item_Turbo_Boy),			1);
		Add(typeof(Item_Twintails),			1);
		Add(typeof(Item_Yayoi),				1);
	}
}