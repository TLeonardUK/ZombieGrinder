// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.scene;
using game.modes.base_game_mode;
using game.statistics.statistic_highest_score;

public class Achievement_Bronze : Achievement
{
	Achievement_Bronze()
	{
		ID 				= "ACHIEVEMENT_BRONZE";
		Name 			= Locale.Get("#achievement_bronze_name");
		Description 	= Locale.Get("#achievement_bronze_description");
		Progress 		= 0;
		Max_Progress 	= 1000000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_015_bronze";
		Track_Statistic	= typeof(Statistic_Highest_Score);
		Use_Statistic_Progress = false;
	}
}