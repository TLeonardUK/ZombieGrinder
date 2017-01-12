// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_human_deaths;

public class Achievement_Failure : Achievement
{
	Achievement_Failure()
	{
		ID 				= "ACHIEVEMENT_FAILURE";
		Name 			= Locale.Get("#achievement_failure_name");
		Description 	= Locale.Get("#achievement_failure_description");
		Progress 		= 0;
		Max_Progress 	= 501;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_002_failure";
		Track_Statistic	= typeof(Statistic_Human_Deaths);
		Use_Statistic_Progress = true;
	}
}