// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_wave;

public class Achievement_ZombieKiller : Achievement
{
	Achievement_ZombieKiller()
	{
		ID 				= "ACHIEVEMENT_ZOMBIEKILLER";
		Name 			= Locale.Get("#achievement_zombiekiller_name");
		Description 	= Locale.Get("#achievement_zombiekiller_description");
		Progress 		= 0;
		Max_Progress 	= 10;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_019_zombiekiller";
		Track_Statistic	= typeof(Statistic_Highest_Wave);
		Use_Statistic_Progress = false;
	}

	default struct Normal
	{
	}
}