// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_human_revives;

public class Achievement_TheGhostIsClear : Achievement
{
	Achievement_TheGhostIsClear()
	{
		ID 				= "ACHIEVEMENT_THEGHOSTISCLEAR";
		Name 			= Locale.Get("#achievement_theghostisclear_name");
		Description 	= Locale.Get("#achievement_theghostisclear_description");
		Progress 		= 0;
		Max_Progress 	= 20;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_041_theghostisclear";
		Track_Statistic	= typeof(Statistic_Human_Revives);
		Use_Statistic_Progress = true;
	}
}