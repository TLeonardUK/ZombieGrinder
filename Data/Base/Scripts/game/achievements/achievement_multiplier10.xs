// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_multiplier;

public class Achievement_Multiplier10 : Achievement
{
	Achievement_Multiplier10()
	{
		ID 				= "ACHIEVEMENT_MULTIPLIER10";
		Name 			= Locale.Get("#achievement_multiplier10_name");
		Description 	= Locale.Get("#achievement_multiplier10_description");
		Progress 		= 0;
		Max_Progress 	= 10;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_050_multiplier10";
		Track_Statistic	= typeof(Statistic_Highest_Multiplier);
		Use_Statistic_Progress = false;
	}
}