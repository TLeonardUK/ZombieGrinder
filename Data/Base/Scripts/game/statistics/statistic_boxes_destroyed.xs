// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Boxes_Destroyed : Statistic
{
	Statistic_Boxes_Destroyed()
	{
		ID 					= "BOXES_DESTROYED";
		Name 				= Locale.Get("#statistic_boxes_destroyed_name");
		Description 		= Locale.Get("#statistic_boxes_destroyed_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Movable_Destroyed(Event_Movable_Destroyed data)
	{
		Value += 1.0f;
	}
}
