// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Aggregated_Level_Ups : Statistic
{
	Statistic_Aggregated_Level_Ups()
	{
		ID 					= "GLOBAL_LEVEL_UPS";
		Name 				= Locale.Get("#statistic_aggregated_level_ups_name");
		Description 		= Locale.Get("#statistic_aggregated_level_ups_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Level_Up()
	{
		Value += 1.0f;
	}
}
