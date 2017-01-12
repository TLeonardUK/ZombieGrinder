// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombies_ignited;

public class Achievement_ItsHotInHere : Achievement
{
	Achievement_ItsHotInHere()
	{
		ID 				= "ACHIEVEMENT_ITSHOTINHERE";
		Name 			= Locale.Get("#achievement_itshotinhere_name");
		Description 	= Locale.Get("#achievement_itshotinhere_description");
		Progress 		= 0;
		Max_Progress 	= 100;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_052_itshotinhere";
		Track_Statistic	= typeof(Statistic_Zombies_Ignited);
		Use_Statistic_Progress = true;
	}
}