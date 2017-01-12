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
public class Challenge_Item_Drop_Table : Drop_Table
{
	Challenge_Item_Drop_Table()
	{
		Drop_Probability = 1.0f; 
		
        // Gems Grab Bags!
		Add(typeof(Item_Small_Gem_Grab_Bag), 30);
		Add(typeof(Item_Medium_Gem_Grab_Bag), 5);
		Add(typeof(Item_Spanner), 			  25);

        // Individual Top-Tier Gems!
		Add(typeof(Item_Aqua_Gem_5),		5);
		Add(typeof(Item_Blue_Gem_5),		5);
		Add(typeof(Item_Green_Gem_5),		5);
		Add(typeof(Item_Orange_Gem_5),		5);
		Add(typeof(Item_Pink_Gem_5),		5);
		Add(typeof(Item_Purple_Gem_5),		5);
		Add(typeof(Item_Red_Gem_5),			5);
		Add(typeof(Item_Yellow_Gem_5),		5);
		Add(typeof(Item_Aqua_Gem_4),		10);
		Add(typeof(Item_Blue_Gem_4),		10);
		Add(typeof(Item_Green_Gem_4),		10);
		Add(typeof(Item_Orange_Gem_4),		10);
		Add(typeof(Item_Pink_Gem_4),		10);
		Add(typeof(Item_Purple_Gem_4),		10);
		Add(typeof(Item_Red_Gem_4),			10);
		Add(typeof(Item_Yellow_Gem_4),		10);
		Add(typeof(Item_Aqua_Gem_3),		15);
		Add(typeof(Item_Blue_Gem_3),		15);
		Add(typeof(Item_Green_Gem_3),		15);
		Add(typeof(Item_Orange_Gem_3),		15);
		Add(typeof(Item_Pink_Gem_3),		15);
		Add(typeof(Item_Purple_Gem_3),		15);
		Add(typeof(Item_Red_Gem_3),			15);
		Add(typeof(Item_Yellow_Gem_3),		15);
        
		// Generic items.
		Add(typeof(Item_Antlers),			5);
		Add(typeof(Item_Armour),			5);
		Add(typeof(Item_Aviators),			5);
		Add(typeof(Item_Bow),				5);
		Add(typeof(Item_Ghetto_Superstar),	5);
		Add(typeof(Item_Glasses),			5);
		Add(typeof(Item_Headset),			5);
		Add(typeof(Item_Scouter),			5);
		Add(typeof(Item_Ami),				5);
		Add(typeof(Item_Shades),			5);
		Add(typeof(Item_Ball_Cap),			5);
		Add(typeof(Item_Beanie_1),			5);
		Add(typeof(Item_Beanie_2),			5);
		Add(typeof(Item_Dantz),				5);
		Add(typeof(Item_Desgardes),			5);
		Add(typeof(Item_Doppleganger),		5);
		Add(typeof(Item_Flat_Top),			5);
		Add(typeof(Item_Fred_Head),			5);
		Add(typeof(Item_Gatsby),			5);
		Add(typeof(Item_Gi_Joe),			5);
		Add(typeof(Item_Golfers_Delight),	5);
		Add(typeof(Item_Little_Sis),		5);
		Add(typeof(Item_Louisiana_Jones),	5);
		Add(typeof(Item_Mami),				5);
		Add(typeof(Item_Nekomimi),			5);
		Add(typeof(Item_Pigtails),			5);
		Add(typeof(Item_Player1hair),		5);
		Add(typeof(Item_Player2hair),		5);
		Add(typeof(Item_Player3hair),		5);
		Add(typeof(Item_Player4hair),		5);
		Add(typeof(Item_Pompador),			5);
		Add(typeof(Item_Poniteru),			5);
		Add(typeof(Item_Pumpkin),			5);
		Add(typeof(Item_Scruffy_Sidekick),	5);
		Add(typeof(Item_Shizue),			5);
		Add(typeof(Item_Skull),				5);
		Add(typeof(Item_Thugpomp),			5);
		Add(typeof(Item_Top_Hat),			5);
		Add(typeof(Item_Trucker),			5);
		Add(typeof(Item_Turbo_Boy),			5);
		Add(typeof(Item_Twintails),			5);
		Add(typeof(Item_Yayoi),				5);

		// Exclusives.
		Add(typeof(Item_Cheer_Bringer),		1);
		Add(typeof(Item_Presento),	   	    1);
	}
}