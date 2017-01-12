// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_rebound_shots;

public class Achievement_Trickshot : Achievement
{
	Achievement_Trickshot()
	{
		ID 				= "ACHIEVEMENT_TRICKSHOT";
		Name 			= Locale.Get("#achievement_trickshot_name");
		Description 	= Locale.Get("#achievement_trickshot_description");
		Progress 		= 0;
		Max_Progress 	= 100;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_025_trickshot";
		Track_Statistic	= typeof(Statistic_Rebound_Shots);
		Use_Statistic_Progress = true;
	}
}