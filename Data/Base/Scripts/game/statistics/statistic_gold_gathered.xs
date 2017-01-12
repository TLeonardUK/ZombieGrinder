// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Gold_Gathered : Statistic
{
	Statistic_Gold_Gathered()
	{
		ID 					= "GOLD_GATHERED";
		Name 				= Locale.Get("#statistic_gold_gathered_name");
		Description 		= Locale.Get("#statistic_gold_gathered_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Coins_Collected(Event_Coins_Collected evt)
	{
		Value += evt.Collected;
	}
}
