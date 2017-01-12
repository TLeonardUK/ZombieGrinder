// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_HP_Movement : Challenge
{
	string[] Possible_Maps = new string[] {	
		"arena_snowbound",
		"wave_cliff",
		"wave_forest"	
	};
	
	int Map_Index = 0;	
	float Alive_For = 0.0f;
	
    Challenge_HP_Movement()
    {
        ID = "CHALLENGE_HP_MOVEMENT";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_hp_movement_description";
        Timeframe = ChallengeTimeframe.Daily;
        Requires_Activation = true;
        Max_Progress = 1.0f;
		Map_Index = 0;
    }

    public override Item_Archetype[] Get_Rewards()
    {
        return Get_Standard_Rewards(2, 2);
    }

    public event void Seeded_Setup()
    {
		Map_Index = Math.Rand(0, Possible_Maps.Length());
		Max_Progress = <int>(Math.Rand(5, 10) * 1000);
        Description = Locale.Get("#challenge_hp_movement_description").Format(new object[] { <object>(<int>Max_Progress), <object>Possible_Maps[Map_Index] });
	}
	
    public event void On_Activate()
    {
		Start_Custom_Map(Possible_Maps[Map_Index], 0x00000000);
    }
	
	public event void On_Tick()
	{
		if (In_Custom_Map)
		{
			Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
			mode.Ignore_Player_Weapons = false;
			mode.No_Bonuses = true;
			
			Actor[] actors = Scene.Find_Actors(typeof(Human_Player));
			foreach (Human_Player player in actors)
			{
                float delta = player.Get_Last_Frame_Movement().Length();
                 if (delta < 0.5f)
                {
                    player.HealNoFX(5.0f * Time.Get_Delta_Seconds(), 0.0f);
                }   
                else if (delta < 32.0f) // Don't damage player if they were teleported.
                {
                    player.HealNoFX(-(delta / 2.0f), 0.0f);
                }         
			//	player.Speed_Multiplier = 2;
				player.Skills_Disabled = true;
			}
			
			int score = mode.Score;
			if (score > Progress)
			{
				Progress = score;
			}
		}
	}
}
