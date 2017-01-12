// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_human_revives;

public class Statistic_Aggregated_Human_Revives : Statistic
{
	Statistic_Aggregated_Human_Revives()
	{
		ID 					= "GLOBAL_HUMAN_REVIVES";
		Name 				= Locale.Get("#statistic_aggregated_human_revives_name");
		Description 		= Locale.Get("#statistic_aggregated_human_revives_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Human_Revives);
	}
}
