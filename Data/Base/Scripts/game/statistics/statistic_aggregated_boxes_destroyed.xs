// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using game.statistics.statistic_boxes_destroyed;

public class Statistic_Aggregated_Boxes_Destroyed : Statistic
{
	Statistic_Aggregated_Boxes_Destroyed()
	{
		ID 					= "GLOBAL_BOXES_DESTROYED";
		Name 				= Locale.Get("#statistic_aggregated_boxes_destroyed_name");
		Description 		= Locale.Get("#statistic_aggregated_boxes_destroyed_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
		Mirror_Statistic	= typeof(Statistic_Boxes_Destroyed);
	}
}
