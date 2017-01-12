// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_zombies_ignited;

public class Statistic_Aggregated_Zombies_Ignited : Statistic
{
	Statistic_Aggregated_Zombies_Ignited()
	{
		ID 					= "GLOBAL_ZOMBIES_IGNITED";
		Name 				= Locale.Get("#statistic_aggregated_zombies_ignited_name");
		Description 		= Locale.Get("#statistic_aggregated_zombies_ignited_description");
		Category			= Locale.Get("#statistic_group_enemies");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Zombies_Ignited);
	}
}
