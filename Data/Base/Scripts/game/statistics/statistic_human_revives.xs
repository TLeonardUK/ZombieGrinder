// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Human_Revives : Statistic
{
	Statistic_Human_Revives()
	{
		ID 					= "HUMAN_REVIVES";
		Name 				= Locale.Get("#statistic_human_revives_name");
		Description 		= Locale.Get("#statistic_human_revives_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	event void On_Pawn_Revived(Event_Pawn_Revived evt)
	{
		if (evt.Reviver != null && evt.Reviver.Owner != null && evt.Reviver.Owner.Is_Local == true)
		{
			Value += 1.0f;
		}
	}
}
