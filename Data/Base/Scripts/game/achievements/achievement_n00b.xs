// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombies_killed;

public class Achievement_n00b : Achievement
{
	Achievement_n00b()
	{
		ID 				= "ACHIEVEMENT_N00B";
		Name 			= Locale.Get("#achievement_n00b_name");
		Description 	= Locale.Get("#achievement_n00b_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_008_n00b";
		Track_Statistic	= typeof(Statistic_Zombies_Killed);
		Use_Statistic_Progress = true;
	}
}