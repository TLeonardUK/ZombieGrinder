// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.engine;

public class Achievement_Mammy : Achievement
{
	Achievement_Mammy()
	{
		ID 				= "ACHIEVEMENT_MAMMY";
		Name 			= Locale.Get("#achievement_mammy_name");
		Description 	= Locale.Get("#achievement_mammy_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_006_mammy";
	}

	default struct Normal
	{
		event void On_Global_Tick()
		{
			if (Engine.Has_Crashed())
			{
				Progress += 1.0f;
			}
		}
	}
}