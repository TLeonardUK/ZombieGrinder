// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_multiplier;

public class Achievement_Multiplier15 : Achievement
{
	Achievement_Multiplier15()
	{
		ID 				= "ACHIEVEMENT_MULTIPLIER15";
		Name 			= Locale.Get("#achievement_multiplier15_name");
		Description 	= Locale.Get("#achievement_multiplier15_description");
		Progress 		= 0;
		Max_Progress 	= 15;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_051_multiplier15";
		Track_Statistic	= typeof(Statistic_Highest_Multiplier);
		Use_Statistic_Progress = false;
	}
}