// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.statistic;
using system.scene;
using game.modes.base_game_mode;
using game.modes.wave;

public class Statistic_Dungeon_Level : Statistic
{
	Statistic_Dungeon_Level()
	{
		ID 					= "DUNGEON_LEVEL";
		Name 				= Locale.Get("#statistic_dungeon_level_name");
		Description 		= Locale.Get("#statistic_dungeon_level_description");
		Category			= Locale.Get("#statistic_group_players");
		Aggregated			= false;
		Aggregation_Period	= 1;
		UI_Display			= true;
		Value				= 0;
		Data_Type			= StatisticDataType.Int;
	}

	public event void On_Tick()
	{
		float score = Scene.Get_Map_Dungeon_Level();
		if (score > Value)
		{
			Value = score;
		}
	}
}
