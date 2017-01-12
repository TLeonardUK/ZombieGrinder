// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_healing_dealt;

public class Achievement_HealingLevel1 : Achievement
{
	Achievement_HealingLevel1()
	{
		ID 				= "ACHIEVEMENT_HEALINGLEVEL1";
		Name 			= Locale.Get("#achievement_healinglevel1_name");
		Description 	= Locale.Get("#achievement_healinglevel1_description");
		Progress 		= 0;
		Max_Progress 	= 10000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_044_healinglevel1";
		Track_Statistic	= typeof(Statistic_Healing_Dealt);
		Use_Statistic_Progress = true;
	}
}