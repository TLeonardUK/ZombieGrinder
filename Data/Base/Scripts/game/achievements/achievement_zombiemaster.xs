// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_wave;

public class Achievement_ZombieMaster : Achievement
{
	Achievement_ZombieMaster()
	{
		ID 				= "ACHIEVEMENT_ZOMBIEMASTER";
		Name 			= Locale.Get("#achievement_zombiemaster_name");
		Description 	= Locale.Get("#achievement_zombiemaster_description");
		Progress 		= 0;
		Max_Progress 	= 20;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_020_zombiemaster";
		Track_Statistic	= typeof(Statistic_Highest_Wave);
		Use_Statistic_Progress = false;
	}


}