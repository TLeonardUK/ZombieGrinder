// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.scene;
using game.modes.base_game_mode;
using game.statistics.statistic_highest_score;

public class Achievement_Silver : Achievement
{
	Achievement_Silver()
	{
		ID 				= "ACHIEVEMENT_SILVER";
		Name 			= Locale.Get("#achievement_silver_name");
		Description 	= Locale.Get("#achievement_silver_description");
		Progress 		= 0;
		Max_Progress 	= 10000000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_016_silver";
		Track_Statistic	= typeof(Statistic_Highest_Score);
		Use_Statistic_Progress = false;
	}
}