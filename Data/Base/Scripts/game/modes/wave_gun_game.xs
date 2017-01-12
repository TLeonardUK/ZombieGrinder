// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.modes.wave;

[
	Placeable(true), 
	Name("Gun Game Wave Mode"), 
	Description("Same as wave mode except you get infinite ammo and your weapon changes each wave.") 
]
public class Wave_Gun_Game_Mode : Wave_Game_Mode
{
	private Type m_weapon_type = null;
	private Weapon m_weapon_type_instance = null;
	private string m_weapon_name = "";

	private Type[] m_weapon_table = 
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

	Wave_Gun_Game_Mode()
	{
		Gem_Drops = true;
		Weapon_Drops = false;
        Allow_Manual_Weapon_Drops = false;

		Gem_Drops_From_Players = false;
		Weapon_Drops_From_Players = false;
		
        Player_Manager.Reset();

		Enemy_Manager.Reset();
		Enemy_Manager.Use_Min_Difficulty_Scalars = true;

		On_Round_Change();
	}

	private Type Select_Random_Weapon()
	{
		Type old_weapon = m_weapon_type;

		while (old_weapon == m_weapon_type)
		{
			old_weapon = m_weapon_table[Math.Rand(0, m_weapon_table.Length())];
		}

		return old_weapon;
	}

	private void Apply_Weapon_Change()
	{
		Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
		foreach (Human_Player actor in potential_revivers)
		{
			// Remove all weapons from actor.
			actor.Remove_All_Weapons();
	
			// Give default pistol to the actor			
			actor.Give_Weapon(m_weapon_type, Vec4(255.0f, 255.0f, 255.0f, 255.0f), true, new int[0], null);
		}
	}

	public override void Setup_Player_Weapons(Pawn actor, Profile profile)
	{		
		// Remove all weapons from actor.
		actor.Remove_All_Weapons();
	
		// Give default pistol to the actor			
		actor.Give_Weapon(m_weapon_type, Vec4(255.0f, 255.0f, 255.0f, 255.0f), true, new int[0], null);
	}

	protected override void On_Round_Change()
	{
		m_weapon_type = Select_Random_Weapon();
		m_weapon_type_instance = <Weapon>m_weapon_type.New();
		m_weapon_name = m_weapon_type_instance.Name;

		Apply_Weapon_Change();
	}
	
	protected override void On_Update_HUD(Game_HUD game_hud, bool new_anim)
	{		
		if (Network.Is_Server())
		{
			m_display_half_time_countdown = <int>Math.Ceiling(m_half_time_countdown / 1000.0f);
			m_display_zombies_remaining = m_zombie_spawns_remaining + Enemy_Manager.Count_Wave_Alive();
		}

		if (m_game_state == Wave_Game_State.Refactory)
		{
			game_hud.Set_Objective_Text(
				Locale.Get("#wave_game_mode_refactory_countdown").Format(new object[] { m_display_half_time_countdown }),
				new_anim);
		}
		else
		{
			game_hud.Set_Objective_Text(
				Locale.Get("#wave_gun_game_mode_active_countdown").Format(new object[] { m_current_round, m_weapon_name, m_display_zombies_remaining }),
				m_use_checkoff_animation);
		}
	}	
	
	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
		m_weapon_name /// eeeeeeeeeeeeeeewh string, fix plz.
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}