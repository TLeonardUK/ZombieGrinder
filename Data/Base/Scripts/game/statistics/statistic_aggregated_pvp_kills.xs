// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_pvp_kills;

public class Statistic_Aggregated_PVP_Kills : Statistic
{
	Statistic_Aggregated_PVP_Kills()
	{
		ID 					= "GLOBAL_PVP_KILLS";
		Name 				= Locale.Get("#statistic_aggregated_pvp_kills_name");
		Description 		= Locale.Get("#statistic_aggregated_pvp_kills_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_PVP_Kills);
	}
}
