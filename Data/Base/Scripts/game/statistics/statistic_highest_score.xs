// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using system.scene;
using game.modes.base_game_mode;

public class Statistic_Highest_Score : Statistic
{
	Statistic_Highest_Score()
	{
		ID 					= "HIGHEST_SCORE";
		Name 				= Locale.Get("#statistic_highest_score_name");
		Description 		= Locale.Get("#statistic_highest_score_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Tick()
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		if (mode != null)
		{
			float score = mode.Score;
			if (score > Value)
			{
				Value = score;
			}
		}
	}
}
