// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_healing_dealt;

public class Achievement_HealingLevel4 : Achievement
{
	Achievement_HealingLevel4()
	{
		ID 				= "ACHIEVEMENT_HEALINGLEVEL4";
		Name 			= Locale.Get("#achievement_healinglevel4_name");
		Description 	= Locale.Get("#achievement_healinglevel4_description");
		Progress 		= 0;
		Max_Progress 	= 1000000;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_047_healinglevel4";
		Track_Statistic	= typeof(Statistic_Healing_Dealt);
		Use_Statistic_Progress = true;
	}
}