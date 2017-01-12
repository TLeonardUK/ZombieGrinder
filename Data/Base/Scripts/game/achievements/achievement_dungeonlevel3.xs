// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_dungeon_level;

public class Achievement_DungeonLevel3 : Achievement
{
	Achievement_DungeonLevel3()
	{
		ID 				= "ACHIEVEMENT_DUNGEONLEVEL3";
		Name 			= Locale.Get("#achievement_dungeonlevel3_name");
		Description 	= Locale.Get("#achievement_dungeonlevel3_description");
		Progress 		= 0;
		Max_Progress 	= 9;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_040_dungeonboss";
		Track_Statistic	= typeof(Statistic_Dungeon_Level);
		Use_Statistic_Progress = true;
	}
}