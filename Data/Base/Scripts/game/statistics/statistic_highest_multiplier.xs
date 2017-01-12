// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using system.scene;
using game.modes.base_game_mode;

public class Statistic_Highest_Multiplier : Statistic
{
	Statistic_Highest_Multiplier()
	{
		ID 					= "HIGHEST_MULTIPLIER";
		Name 				= Locale.Get("#statistic_highest_multiplier_name");
		Description 		= Locale.Get("#statistic_highest_multiplier_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Float;
	}

	public event void On_Tick()
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		if (mode != null)
		{
			float multiplier = mode.Multiplier;
			if (multiplier > Value)
			{
				Value = multiplier;
			}
		}
	}
}
