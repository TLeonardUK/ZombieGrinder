// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Zombies_Shocked : Statistic
{
	Statistic_Zombies_Shocked()
	{
		ID 					= "ZOMBIES_SHOCKED";
		Name 				= Locale.Get("#statistic_zombies_shocked_name");
		Description 		= Locale.Get("#statistic_zombies_shocked_description");
		Category			= Locale.Get("#statistic_group_enemies");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Pawn_Status_Ailment(Event_Pawn_Status_Ailment evt)
	{
		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Instigator;
		NetUser killer_user  = killer_actor == null ? null : killer_actor.Owner;

		if (killer_actor != null)
		{
			NetUser imposer = killer_actor.Get_Imposing_As();
			if (imposer != null)
			{
				killer_user  = imposer;			
				killer_actor = null;
			}
		}

		// Give rewards to local players.
		if (killer_user != null && killer_user.Is_Local == true)
		{
			if ((<Enemy>evt.Effected) != null && evt.Ailment == StatusAilmentType.Shocked)
			{
				Value += 1.0f;
			}
		}
	}
}
