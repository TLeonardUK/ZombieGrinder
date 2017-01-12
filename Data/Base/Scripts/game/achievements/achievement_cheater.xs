// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_Cheater : Achievement
{
	Achievement_Cheater()
	{
		ID 				= "ACHIEVEMENT_CHEATER";
		Name 			= Locale.Get("#achievement_cheater_name");
		Description 	= Locale.Get("#achievement_cheater_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_028_cheater";
	}

	default struct Normal
	{
        event void On_Global_Tick()
		{
			if (Engine.Get_Highest_Profile_Level() >= 99)
			{
				Progress += 1.0f;
			}
		}
	}
}