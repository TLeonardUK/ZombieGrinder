// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;

public class Statistic_Aggregated_Characters_Created : Statistic
{
	Statistic_Aggregated_Characters_Created()
	{
		ID 					= "GLOBAL_CHARACTERS_CREATED";
		Name 				= Locale.Get("#statistic_aggregated_characters_created_name");
		Description 		= Locale.Get("#statistic_aggregated_characters_created_description");
		Category			= Locale.Get("#statistic_group_general");
		Aggregated			= true;
		Aggregation_Period	= 0;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Character_Created()
	{
		Value += 1.0f;
	}
}
