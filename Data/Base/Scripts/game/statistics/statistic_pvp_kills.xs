// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_PVP_Kills : Statistic
{
	Statistic_PVP_Kills()
	{
		ID 					= "PVP_KILLS";
		Name 				= Locale.Get("#statistic_pvp_kills_name");
		Description 		= Locale.Get("#statistic_pvp_kills_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}
	
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
				killer_actor = null;
			}
		}

		// Ignore self-kills.
		if (killer_actor == evt.Dead)
		{
			return;
		}

		// Give rewards to local players.
		if (killer_user != null && 
			killer_user.Is_Local == true)
		{
			if ((<Human_Player>evt.Dead) != null)
			{
				Value += 1.0f;
			}
		}
	}
}
