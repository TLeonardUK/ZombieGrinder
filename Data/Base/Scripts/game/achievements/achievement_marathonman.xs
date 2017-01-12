// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_pixels_moved;

public class Achievement_MarathonMan : Achievement
{
	Achievement_MarathonMan()
	{
		ID 				= "ACHIEVEMENT_MARATHONMAN";
		Name 			= Locale.Get("#achievement_marathonman_name");
		Description 	= Locale.Get("#achievement_marathonman_description");
		Progress 		= 0;
		Max_Progress 	= 1000000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_010_marathonman";
		Track_Statistic	= typeof(Statistic_Pixels_Moved);
		Use_Statistic_Progress = true;
	}
}