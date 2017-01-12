// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;
using system.time;

public class Achievement_Workshop : Achievement
{
	Achievement_Workshop()
	{
		ID 				= "ACHIEVEMENT_WORKSHOP";
		Name 			= Locale.Get("#achievement_workshop_name");
		Description 	= Locale.Get("#achievement_workshop_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_053_workshop";
	}

	default struct Normal
	{
		event void On_Global_Tick()
		{
			if (Engine.Has_Uploaded_To_Workshop())
			{
				Progress += 1.0f;
			}
		}
	}
}