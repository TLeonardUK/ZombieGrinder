// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.challenge;

public class Challenge_Weapon_Switch : Challenge
{
	string[] Possible_Maps = new string[] {	
		"arena_snowbound",
		"arena_circle",
		"wave_cliff",
		"wave_forest",
		"wave_crowd",
		"wave_tiny"
	};
	
	private Type[] Weapon_Table = 
	{
		typeof(Weapon_Flamethrower),
		typeof(Weapon_Grenade_Launcher),
		typeof(Weapon_Rocket_Launcher),
		typeof(Weapon_RPC),
		typeof(Weapon_Sticky_Launcher),
		typeof(Weapon_Baseball_Bat),
		typeof(Weapon_Fight_Saber),
		typeof(Weapon_Flare_Gun),
		typeof(Weapon_Magnum),
		typeof(Weapon_Pistol),
		typeof(Weapon_Ak47),
		typeof(Weapon_Assault_Rifle),
		typeof(Weapon_Winchester_Rifle),
		typeof(Weapon_DB_Shotgun),
		typeof(Weapon_Scatter_Gun),
		typeof(Weapon_Shotgun),
		typeof(Weapon_Spaz),
		typeof(Weapon_Uzi),
		typeof(Weapon_Rocket_Launcher_2),
		typeof(Weapon_Rocket_Launcher_3),
		typeof(Weapon_Frag),
		typeof(Weapon_Molotov),
		typeof(Weapon_Sword)
	};

	int Map_Index = 0;	

    float m_switch_timer = 0.0f;
    Type m_weapon_type = null;

    const int SWITCH_INTERVAL = 10.0f;
	
    Challenge_Weapon_Switch()
    {
        ID = "CHALLENGE_WEAPON_SWITCH";
        Name = "#menu_game_setup_challenge_daily";
        Description = "#challenge_weapon_switch_description";
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
        Description = Locale.Get("#challenge_weapon_switch_description").Format(new object[] { <object>(<int>Max_Progress), <object>Possible_Maps[Map_Index] });
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
			if (mode)
			{
                mode.Weapon_Drops = false;

                m_switch_timer += Time.Get_Delta_Seconds();

               // Log.WriteOnScreen("WEAPON_SWITCH", Vec4(1.0f, 0.0f, 0.0f, 1.0f), 1.0f, "Timer="+(SWITCH_INTERVAL-m_switch_timer));
                
                if (m_switch_timer >= SWITCH_INTERVAL)
                {
                    m_weapon_type = Select_Random_Weapon();

		            Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
		            foreach (Human_Player actor in potential_revivers)
		            {
			            actor.Remove_All_Weapons();	
			            actor.Give_Weapon(m_weapon_type, Vec4(255.0f, 255.0f, 255.0f, 255.0f), true, new int[0], null);
		            }

                    m_switch_timer = 0.0f;
                }

		    	Wave_Game_Mode wave_mode = <Wave_Game_Mode>Scene.Active_Game_Mode;
                if (wave_mode)
                {
				    if (wave_mode.Get_Wave() >= Progress)
				    {
					    Progress = wave_mode.Get_Wave();			
				    }
                }
			}
		}
	}

	private Type Select_Random_Weapon()
	{
		Type old_weapon = m_weapon_type;

		while (old_weapon == m_weapon_type)
		{
			old_weapon = Weapon_Table[Math.Rand(0, Weapon_Table.Length())];
		}

		return old_weapon;
	}
}
