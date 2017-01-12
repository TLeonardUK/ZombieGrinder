// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_dungeon_level;

public class Achievement_DungeonLevel1 : Achievement
{
	Achievement_DungeonLevel1()
	{
		ID 				= "ACHIEVEMENT_DUNGEONLEVEL1";
		Name 			= Locale.Get("#achievement_dungeonlevel1_name");
		Description 	= Locale.Get("#achievement_dungeonlevel1_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_038_dungeoncrawler";
		Track_Statistic	= typeof(Statistic_Dungeon_Level);
		Use_Statistic_Progress = true;
	}
}