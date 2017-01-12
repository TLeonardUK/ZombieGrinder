// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Human_Deaths : Statistic
{
	Statistic_Human_Deaths()
	{
		ID 					= "HUMAN_DEATHS";
		Name 				= Locale.Get("#statistic_human_deaths_name");
		Description 		= Locale.Get("#statistic_human_deaths_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	event void On_Pawn_Killed(Event_Pawn_Killed evt)
	{		
		// Give rewards to local players.
		if (evt.Dead.Owner != null && evt.Dead.Owner.Is_Local == true)
		{
			Value += 1.0f;
		}
	}
}
