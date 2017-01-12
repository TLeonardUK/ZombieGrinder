// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_healing_dealt;

public class Achievement_HealingLevel2 : Achievement
{
	Achievement_HealingLevel2()
	{
		ID 				= "ACHIEVEMENT_HEALINGLEVEL2";
		Name 			= Locale.Get("#achievement_healinglevel2_name");
		Description 	= Locale.Get("#achievement_healinglevel2_description");
		Progress 		= 0;
		Max_Progress 	= 50000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_045_healinglevel2";
		Track_Statistic	= typeof(Statistic_Healing_Dealt);
		Use_Statistic_Progress = true;
	}
}