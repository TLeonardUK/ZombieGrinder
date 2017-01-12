// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_EagleEye : Achievement
{
	private int m_shots_fired = 0;

	Achievement_EagleEye()
	{
		ID 				= "ACHIEVEMENT_EAGLEEYE";
		Name 			= Locale.Get("#achievement_eagleeye_name");
		Description 	= Locale.Get("#achievement_eagleeye_description");
		Progress 		= 0;
		Max_Progress 	= 50;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_022_eagleeye";
	}
	
	public event void On_Pawn_Hurt(Event_Pawn_Hurt evt)
	{
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
			if (evt.Damage > 1.0f)
			{
				Progress = 0.0f;
			}
		}
	}

	public event void On_Tick()
	{		
		NetUser[] users = Network.Get_Local_Users();	
		foreach (NetUser user in users)
		{			
			Controller controller = user.Controller;
			if (controller != null)
			{
				Pawn pawn = controller.Possessed;
				if (pawn != null)
				{
					Progress += pawn.Get_Shots_Fired_Delta();
				}
			}
		}
	}
}