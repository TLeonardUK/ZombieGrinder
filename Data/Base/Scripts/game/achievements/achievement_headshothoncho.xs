// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombie_headshots;

public class Achievement_HeadshotHoncho : Achievement
{
	private int m_headshot_counter = 0;

	Achievement_HeadshotHoncho()
	{
		ID 				= "ACHIEVEMENT_HEADSHOTHONCHO";
		Name 			= Locale.Get("#achievement_headshothoncho_name");
		Description 	= Locale.Get("#achievement_headshothoncho_description");
		Progress 		= 0;
		Max_Progress 	= 100;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_005_headshothoncho";
		Track_Statistic	= typeof(Statistic_Zombie_Headshots);
		Use_Statistic_Progress = true;
	}
}