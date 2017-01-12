// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Highest_Rank : Statistic
{
	Statistic_Highest_Rank()
	{
		ID 					= "HIGHEST_RANK";
		Name 				= Locale.Get("#statistic_highest_rank_name");
		Description 		= Locale.Get("#statistic_highest_rank_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Rank_Updated(int new_rank)
	{
		if (new_rank > Value)
		{
			Value = new_rank;
		}
	}
}
