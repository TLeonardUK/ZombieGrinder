// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using game.statistics.statistic_pvp_kills;

public class Achievement_Owned : Achievement
{
	Achievement_Owned()
	{
		ID 				= "ACHIEVEMENT_OWNED";
		Name 			= Locale.Get("#achievement_owned_name");
		Description 	= Locale.Get("#achievement_owned_description");
		Progress 		= 0;
		Max_Progress 	= 10;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_027_rapist";
		Track_Statistic	= typeof(Statistic_PVP_Kills);
		Use_Statistic_Progress = true;
	}
}