// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_human_deaths;

public class Statistic_Aggregated_Human_Deaths : Statistic
{
	Statistic_Aggregated_Human_Deaths()
	{
		ID 					= "GLOBAL_HUMAN_DEATHS";
		Name 				= Locale.Get("#statistic_aggregated_human_deaths_name");
		Description 		= Locale.Get("#statistic_aggregated_human_deaths_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Human_Deaths);
	}
}
