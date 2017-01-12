// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_Penetrator : Achievement
{
	Achievement_Penetrator()
	{
		ID 				= "ACHIEVEMENT_PENETRATOR";
		Name 			= Locale.Get("#achievement_penetrator_name");
		Description 	= Locale.Get("#achievement_penetrator_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_007_penetrator";
	}

	public event void On_Pawn_Hurt(Event_Pawn_Hurt evt)
	{
		// Enough penetrations?
		if (evt.Hurt.Last_Damage_Penetration_Count < 5)
		{
			return;
		}

		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Harmer;
		NetUser killer_user  = killer_actor == null ? null : killer_actor.Owner;

		if (killer_actor != null)
		{
			NetUser imposer = killer_actor.Get_Imposing_As();
			if (imposer != null)
			{
				killer_user = imposer;			
				killer_actor = null;
			}
		}

		// Give rewards to local players.
		if (killer_user != null && 
			killer_user.Is_Local == true)
		{
			Progress += 1.0f;
		}
	}
}