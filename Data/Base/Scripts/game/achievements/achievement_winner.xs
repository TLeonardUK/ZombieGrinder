// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_rank;

public class Achievement_Winner : Achievement
{
	Achievement_Winner()
	{
		ID 				= "ACHIEVEMENT_WINNER";
		Name 			= Locale.Get("#achievement_winner_name");
		Description 	= Locale.Get("#achievement_winner_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_048_winner";
		Track_Statistic	= typeof(Statistic_Highest_Rank);
		Use_Statistic_Progress = false;
	}
}