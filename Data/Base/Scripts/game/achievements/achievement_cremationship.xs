// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_long_distance_ignite;

public class Achievement_Cremationship : Achievement
{
	Achievement_Cremationship()
	{
		ID 				= "ACHIEVEMENT_CREMATIONSHIP";
		Name 			= Locale.Get("#achievement_cremationship_name");
		Description 	= Locale.Get("#achievement_cremationship_description");
		Progress 		= 0;
		Max_Progress 	= 100;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_011_cremationship";
		Track_Statistic	= typeof(Statistic_Long_Distance_Ignite);
		Use_Statistic_Progress = true;
	}
}