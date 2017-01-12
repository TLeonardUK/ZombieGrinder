// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.time;

public class Achievement_YouDidntCrash : Achievement
{
	Achievement_YouDidntCrash()
	{
		ID 				= "ACHIEVEMENT_YOUDIDNTCRASH";
		Name 			= Locale.Get("#achievement_youdidntcrash_name");
		Description 	= Locale.Get("#achievement_youdidntcrash_description");
		Progress 		= 0;
		Max_Progress 	= 60 * 60;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_026_youdidntcrash";
	}

	default struct Normal
	{
		public event void On_Enter()
		{
			Progress = 0.0f;
		}
		public event void On_Global_Tick()
		{
			Progress += Time.Get_Delta_Seconds();
		}
	}
}