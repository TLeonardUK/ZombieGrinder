// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_multiplier;

public class Achievement_Multiplier5 : Achievement
{
	Achievement_Multiplier5()
	{
		ID 				= "ACHIEVEMENT_MULTIPLIER5";
		Name 			= Locale.Get("#achievement_multiplier5_name");
		Description 	= Locale.Get("#achievement_multiplier5_description");
		Progress 		= 0;
		Max_Progress 	= 5;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_049_multiplier5";
		Track_Statistic	= typeof(Statistic_Highest_Multiplier);
		Use_Statistic_Progress = false;
	}
}