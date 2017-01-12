// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Play_Time : Statistic
{
	Statistic_Play_Time()
	{
		ID 					= "PLAY_TIME";
		Name 				= Locale.Get("#statistic_play_time_name");
		Description 		= Locale.Get("#statistic_play_time_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Float;
	}

	default state Normal
	{
		public event void On_Global_Tick()
		{
			Value += ((Time.Get_Delta_Seconds() / 60.0f) / 60.0f);
		}
	}
}
