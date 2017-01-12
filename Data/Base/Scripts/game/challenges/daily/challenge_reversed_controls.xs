// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Reversed_Controls : Challenge
{
	string[] Possible_Maps = new string[] {	
		"arena_snowbound",
		"arena_circle",
		"wave_cliff",
		"wave_forest",
		"wave_crowd",
		"wave_tiny"
	};
	
	int Map_Index = 0;	
	
    Challenge_Reversed_Controls()
    {
        ID = "CHALLENGE_REVERSED_CONTROLS";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_reversed_controls_description";
        Timeframe = ChallengeTimeframe.Daily;
        Requires_Activation = true;
        Max_Progress = 1.0f;
		Map_Index = 0;
    }

    public override Item_Archetype[] Get_Rewards()
    {
		int MaxRewards = Math.Min(4, 1 + ((<int>Max_Progress - 20) / 10));
        return Get_Standard_Rewards(1, MaxRewards);
    }

    public event void Seeded_Setup()
    {
		Map_Index = Math.Rand(0, Possible_Maps.Length());
		Max_Progress = <int>(Math.Rand(2, 6) * 10);
        Description = Locale.Get("#challenge_reversed_controls_description").Format(new object[] { <object>(<int>Max_Progress), <object>Possible_Maps[Map_Index] });
	}
	
    public event void On_Activate()
    {
		Start_Custom_Map(Possible_Maps[Map_Index], 0x00000000);
    }
	
	public event void On_Tick()
	{
		if (In_Custom_Map)
		{
		    Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
		    foreach (Human_Player actor in potential_revivers)
		    {
                actor.Reversed_Controls = true;
		    }
		}
	}
}
