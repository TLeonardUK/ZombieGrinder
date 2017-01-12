// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_DeveloperDeath : Achievement
{
	Achievement_DeveloperDeath()
	{
		ID 				= "ACHIEVEMENT_DEVELOPERDEATH";
		Name 			= Locale.Get("#achievement_developerdeath_name");
		Description 	= Locale.Get("#achievement_developerdeath_description");
		Progress 		= 0;
		Max_Progress 	= 1;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_042_developerdeath";
	}

	default struct Normal
	{
		event void On_Pawn_Killed(Event_Pawn_Killed evt)
		{		
			// Deal with imposing-actors (turrets etc).
			Actor   killer_actor = evt.Killer;
			NetUser killer_user  = killer_actor == null ? null : killer_actor.Owner;

			if (killer_actor != null)
			{
				NetUser imposer = killer_actor.Get_Imposing_As();
				if (imposer != null)
				{
					killer_user = imposer;	
				}
			}

			// Progress!
			if (killer_user != null && killer_user.Is_Local == true)
			{
				if (evt.Dead != null && evt.Dead.Owner != null)
				{
					if (evt.Dead.Owner.First_Party_ID == "76561197977204045" ||  /* Infinitus */
						evt.Dead.Owner.First_Party_ID == "76561197998740683")	 /* Jordizzle */
					{
						Progress += 1.0f;
					}
				}
			}
		}
	}
}