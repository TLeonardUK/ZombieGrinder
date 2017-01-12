// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_highest_wave;

public class Achievement_CantTouchThis : Achievement
{
	Achievement_CantTouchThis()
	{
		ID 				= "ACHIEVEMENT_CANTTOUCHTHIS";
		Name 			= Locale.Get("#achievement_canttouchthis_name");
		Description 	= Locale.Get("#achievement_canttouchthis_description");
		Progress 		= 0;
		Max_Progress 	= 50;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_029_canttouchthis";
		Track_Statistic	= typeof(Statistic_Highest_Wave);
		Use_Statistic_Progress = false;
	}
}