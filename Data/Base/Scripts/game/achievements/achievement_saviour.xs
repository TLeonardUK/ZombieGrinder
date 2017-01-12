// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_human_revives;

public class Achievement_Saviour : Achievement
{
	Achievement_Saviour()
	{
		ID 				= "ACHIEVEMENT_SAVIOUR";
		Name 			= Locale.Get("#achievement_saviour_name");
		Description 	= Locale.Get("#achievement_saviour_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_003_saviour";
		Track_Statistic	= typeof(Statistic_Human_Revives);
		Use_Statistic_Progress = true;
	}
}