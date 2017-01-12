// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_zombies_frozen_ignited;

public class Achievement_IceAndFire : Achievement
{
	Achievement_IceAndFire()
	{
		ID 				= "ACHIEVEMENT_ICEANDFIRE";
		Name 			= Locale.Get("#achievement_iceandfire_name");
		Description 	= Locale.Get("#achievement_iceandfire_description");
		Progress 		= 0;
		Max_Progress 	= 5;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_018_iceandfire";
		Track_Statistic	= typeof(Statistic_Zombies_Frozen_Ignited);
		Use_Statistic_Progress = true;
	}
}