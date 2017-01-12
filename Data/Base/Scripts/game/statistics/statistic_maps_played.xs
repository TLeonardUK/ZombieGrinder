// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Maps_Played : Statistic
{
	Statistic_Maps_Played()
	{
		ID 					= "MAPS_PLAYED";
		Name 				= Locale.Get("#statistic_maps_played_name");
		Description 		= Locale.Get("#statistic_maps_played_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Map_Load()
	{
		Value += 1.0f;
	}
}
