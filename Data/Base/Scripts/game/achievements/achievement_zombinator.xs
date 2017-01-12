// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombies_killed;

public class Achievement_Zombinator : Achievement
{
	Achievement_Zombinator()
	{
		ID 				= "ACHIEVEMENT_ZOMBINATOR";
		Name 			= Locale.Get("#achievement_zombinator_name");
		Description 	= Locale.Get("#achievement_zombinator_description");
		Progress 		= 0;
		Max_Progress 	= 53598;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_001_zombinator";		
		Track_Statistic	= typeof(Statistic_Zombies_Killed);
		Use_Statistic_Progress = true;
	}
}