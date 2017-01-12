// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_HP_Bullets : Challenge
{
	string[] Possible_Maps = new string[] {	
		"arena_snowbound",
		"wave_cliff",
		"wave_forest"	
	};
	
	int Map_Index = 0;	
	float Alive_For = 0.0f;
	
    Challenge_HP_Bullets()
    {
        ID = "CHALLENGE_HP_BULLETS";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_hp_bullets_description";
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
        Description = Locale.Get("#challenge_hp_bullets_description").Format(new object[] { <object>(<int>Max_Progress), <object>Possible_Maps[Map_Index] });
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
				Weapon weapon = player.Get_Active_Weapon();
				if (weapon != null)
				{
					int fired = weapon.Get_Shots_Fired_Delta();
					if (fired > 0)
					{
						player.Heal(-((player.Max_Health / 50.0f) * fired), 0.0f);
					}
				}
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
