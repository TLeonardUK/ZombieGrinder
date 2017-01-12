// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.scene;
using game.modes.base_game_mode;
using game.statistics.statistic_highest_score;

public class Achievement_Gold : Achievement
{
	Achievement_Gold()
	{
		ID 				= "ACHIEVEMENT_GOLD";
		Name 			= Locale.Get("#achievement_gold_name");
		Description 	= Locale.Get("#achievement_gold_description");
		Progress 		= 0;
		Max_Progress 	= 100000000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_017_gold";
		Track_Statistic	= typeof(Statistic_Highest_Score);
		Use_Statistic_Progress = false;
	}
}