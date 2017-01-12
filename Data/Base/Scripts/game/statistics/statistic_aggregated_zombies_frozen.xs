// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_zombies_frozen;

public class Statistic_Aggregated_Zombies_Frozen : Statistic
{
	Statistic_Aggregated_Zombies_Frozen()
	{
		ID 					= "GLOBAL_ZOMBIES_FROZEN";
		Name 				= Locale.Get("#statistic_aggregated_zombies_frozen_name");
		Description 		= Locale.Get("#statistic_aggregated_zombies_frozen_description");
		Category			= Locale.Get("#statistic_group_enemies");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Zombies_Frozen);
	}
}
