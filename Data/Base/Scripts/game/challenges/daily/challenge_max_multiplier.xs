// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Max_Multiplier : Challenge
{
	string[] Possible_Maps = new string[] {	
		"arena_snowbound",
		"arena_circle",
		"wave_cliff",
		"wave_forest",
		"ggwave_urban",
		"wave_crowd",
		"wave_tiny"
	};
	
	int Map_Index = 0;	
	
    Challenge_Max_Multiplier()
    {
        ID = "CHALLENGE_MAX_MULTIPLIER";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_max_multiplier_description";
        Timeframe = ChallengeTimeframe.Daily;
        Requires_Activation = true;
        Max_Progress = 1.0f;
		Map_Index = 0;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int MaxRewards = Math.Min(4, 1 + ((<int>Max_Progress - 5) / 5));
        return Get_Standard_Rewards(1, MaxRewards);
    }

    public event void Seeded_Setup()
    {
		Map_Index = Math.Rand(0, Possible_Maps.Length());
		Max_Progress = <int>(Math.Rand(5, 15));
        Description = Locale.Get("#challenge_max_multiplier_description").Format(new object[] { <object>(<int>Max_Progress), <object>Possible_Maps[Map_Index] });
	}
	
    public event void On_Activate()
    {
		Start_Custom_Map(Possible_Maps[Map_Index], 0x00000000);
    }
	
	public event void On_Tick()
	{
		if (In_Custom_Map)
		{
			Wave_Game_Mode mode = <Wave_Game_Mode>Scene.Active_Game_Mode;
			if (mode)
			{
				if (mode.Multiplier >= Progress)
				{
					Progress = mode.Multiplier;			
				}
			}
		}
	}
}
