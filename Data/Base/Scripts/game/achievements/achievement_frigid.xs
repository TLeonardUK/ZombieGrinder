// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombies_frozen;

public class Achievement_Frigid : Achievement
{
	Achievement_Frigid()
	{
		ID 				= "ACHIEVEMENT_FRIGID";
		Name 			= Locale.Get("#achievement_frigid_name");
		Description 	= Locale.Get("#achievement_frigid_description");
		Progress 		= 0;
		Max_Progress 	= 100;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_012_frigid";
		Track_Statistic	= typeof(Statistic_Zombies_Frozen);
		Use_Statistic_Progress = true;
	}
}