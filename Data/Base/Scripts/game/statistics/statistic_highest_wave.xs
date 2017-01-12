// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using system.scene;
using game.modes.base_game_mode;
using game.modes.wave;

public class Statistic_Highest_Wave : Statistic
{
	Statistic_Highest_Wave()
	{
		ID 					= "HIGHEST_WAVE";
		Name 				= Locale.Get("#statistic_highest_wave_name");
		Description 		= Locale.Get("#statistic_highest_wave_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Tick()
	{
		Wave_Game_Mode mode = <Wave_Game_Mode>Scene.Active_Game_Mode;
		if (mode != null)
		{
			float score = mode.Get_Wave();
			if (score > Value)
			{
				Value = score;
			}
		}
	}
}
