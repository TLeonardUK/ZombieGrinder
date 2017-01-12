// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_Horder : Achievement
{
	Achievement_Horder()
	{
		ID 				= "ACHIEVEMENT_HORDER";
		Name 			= Locale.Get("#achievement_horder_name");
		Description 	= Locale.Get("#achievement_horder_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_023_oniomania";
	}

	default struct Normal
	{
		event void On_Tick()
		{
			NetUser[] users = Network.Get_Local_Users();
			foreach (NetUser user in users)
			{
				if (user.Local_Profile.Coins >= 500000)
				{
					Progress += 1.0f;
				}
			}
		}
	}
}