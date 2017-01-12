// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_gold_gathered;

public class Statistic_Aggregated_Gold_Gathered : Statistic
{
	Statistic_Aggregated_Gold_Gathered()
	{
		ID 					= "GLOBAL_GOLD_GATHERED";
		Name 				= Locale.Get("#statistic_aggregated_gold_gathered_name");
		Description 		= Locale.Get("#statistic_aggregated_gold_gathered_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Gold_Gathered);
	}
}
