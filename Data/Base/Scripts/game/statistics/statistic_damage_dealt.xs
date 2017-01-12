// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Damage_Dealt : Statistic
{
	Statistic_Damage_Dealt()
	{
		ID 					= "DAMAGE_DEALT";
		Name 				= Locale.Get("#statistic_damage_dealt_name");
		Description 		= Locale.Get("#statistic_damage_dealt_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
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
				Value += evt.Damage;
			}
		}
	}
}
