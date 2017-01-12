// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_maps_played;

public class Statistic_Aggregated_Maps_Played : Statistic
{
	Statistic_Aggregated_Maps_Played()
	{
		ID 					= "GLOBAL_MAPS_PLAYED";
		Name 				= Locale.Get("#statistic_aggregated_maps_played_name");
		Description 		= Locale.Get("#statistic_aggregated_maps_played_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Maps_Played);
	}
}
