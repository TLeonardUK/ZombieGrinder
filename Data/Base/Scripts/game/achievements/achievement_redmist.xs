// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.achievement;

public class Achievement_RedMist : Achievement
{
	private int m_explosion_kills = 0;

	Achievement_RedMist()
	{
		ID 				= "ACHIEVEMENT_REDMIST";
		Name 			= Locale.Get("#achievement_redmist_name");
		Description 	= Locale.Get("#achievement_redmist_description");
		Progress 		= 0;
		Max_Progress 	= 3;
		Locked_Frame 	= "achievement_locked";
		Unlocked_Frame 	= "achievement_013_redmist";
	}

	public void On_Tick()
	{
		m_explosion_kills = 0;
	}

	event void On_Pawn_Killed(Event_Pawn_Killed evt)
	{		
		// Explosion?
		if (evt.Dead.Last_Damage_Type != CollisionDamageType.Explosive)
		{
			return;
		}

		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Killer;
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
			if ((<Enemy>evt.Dead) != null)
			{
				m_explosion_kills++;

				if (m_explosion_kills >= Max_Progress)
				{
					Progress = Max_Progress;
				}
			}
		}
	}
}