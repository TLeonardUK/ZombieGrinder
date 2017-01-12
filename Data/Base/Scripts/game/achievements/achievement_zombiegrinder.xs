// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_wave;

public class Achievement_ZombieGrinder : Achievement
{
	Achievement_ZombieGrinder()
	{
		ID 				= "ACHIEVEMENT_ZOMBIEGRINDER";
		Name 			= Locale.Get("#achievement_zombiegrinder_name");
		Description 	= Locale.Get("#achievement_zombiegrinder_description");
		Progress 		= 0;
		Max_Progress 	= 30;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_021_zombiegrinder";
		Track_Statistic	= typeof(Statistic_Highest_Wave);
		Use_Statistic_Progress = false;
	}
}