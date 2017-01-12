// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_healing_dealt;

public class Achievement_HealingLevel3 : Achievement
{
	Achievement_HealingLevel3()
	{
		ID 				= "ACHIEVEMENT_HEALINGLEVEL3";
		Name 			= Locale.Get("#achievement_healinglevel3_name");
		Description 	= Locale.Get("#achievement_healinglevel3_description");
		Progress 		= 0;
		Max_Progress 	= 100000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_046_healinglevel3";
		Track_Statistic	= typeof(Statistic_Healing_Dealt);
		Use_Statistic_Progress = true;
	}
}