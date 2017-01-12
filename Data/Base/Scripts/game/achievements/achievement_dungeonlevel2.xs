// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_dungeon_level;

public class Achievement_DungeonLevel2 : Achievement
{
	Achievement_DungeonLevel2()
	{
		ID 				= "ACHIEVEMENT_DUNGEONLEVEL2";
		Name 			= Locale.Get("#achievement_dungeonlevel2_name");
		Description 	= Locale.Get("#achievement_dungeonlevel2_description");
		Progress 		= 0;
		Max_Progress 	= 4;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_039_dungeonmaster";
		Track_Statistic	= typeof(Statistic_Dungeon_Level);
		Use_Statistic_Progress = true;
	}
}