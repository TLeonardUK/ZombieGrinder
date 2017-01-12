// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_pvp_kills;

public class Achievement_Dick : Achievement
{
	Achievement_Dick()
	{
		ID 				= "ACHIEVEMENT_DICK";
		Name 			= Locale.Get("#achievement_dick_name");
		Description 	= Locale.Get("#achievement_dick_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_009_dick";
		Track_Statistic	= typeof(Statistic_PVP_Kills);
		Use_Statistic_Progress = true;
	}
}