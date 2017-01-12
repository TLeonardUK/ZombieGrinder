// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.game_mode;
using system.time;
using system.network;
using system.audio;
using game.modes.huds.game_hud;
using game.modes.base_game_mode;
using game.difficulty.difficulty_manager;
using runtime.math;
using runtime.log;
using game.actors.player.player;
using game.actors.player.player_manager;
using game.actors.cameras.zoom_fit_camera;
using game.actors.enemies.enemy_manager;

public enum Wave_Game_State
{
	In_Progress		= 0,
	Refactory		= 1,
	Game_Over		= 2
}

[
	Placeable(true), 
	Name("Wave"), 
	Description("Typical wave based game mode. Zombies, break, zombies, break, ad infinum.") 
]
public class Wave_Game_Mode : Base_Game_Mode
{ 
	// -------------------------------------------------------------------------------------------------
	// Fields.
	// -------------------------------------------------------------------------------------------------
	protected int				m_current_round;	
	protected float 			m_half_time_countdown;
	protected int				m_zombie_spawns_remaining;

	protected bool				m_use_checkoff_animation;

	protected bool				m_increased_round;
		
	protected int				m_display_half_time_countdown;
	protected int				m_display_zombies_remaining;

	protected int				m_started = false;

	protected bool				m_humans_have_spawned = false;

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------
	protected Wave_Game_State	m_game_state;

	// -------------------------------------------------------------------------------------------------
	// State modifiers.
	// -------------------------------------------------------------------------------------------------
	const float Half_Time_Minimum_Duration		= 5000;
	const float Half_Time_Duration				= 10000;

	const float Enemies_Per_Round				= 15;			// Scales with difficulty.
	
	// -------------------------------------------------------------------------------------------------
	// Gameplay modifiers.
	// -------------------------------------------------------------------------------------------------
	public serialized(10) bool Spawn_Fodder			= false;
	public serialized(10) bool Spawn_Chaser			= true;
	public serialized(10) bool Spawn_Exploder		= true;
	public serialized(10) bool Spawn_Bouncer		= true;
	public serialized(200) bool Spawn_Small_Blobby	= true;
	public serialized(200) bool Spawn_Large_Blobby	= true;
	public serialized(700) bool Spawn_Floater		= true;

	// -------------------------------------------------------------------------------------------------
	// Wave modifiers.
	// -------------------------------------------------------------------------------------------------
	// normal
	// normal
	// normal
	// normal
	// all exploders
	// normal
	// normal
	// normal
	// all exploders
	// ???

	// -------------------------------------------------------------------------------------------------
	// Derived properties. Used by native code to interact with the script.
	// -------------------------------------------------------------------------------------------------
	public override bool Is_PVP()
	{
		return false;
	}
	
	public override string[] Get_Scoreboard_Columns()
	{
		return 
		{ 
			"#scoreboard_column_coins", 
			"#scoreboard_column_kills", 
			"#scoreboard_column_deaths",
			"#scoreboard_column_revives"
		};
	}
	
	public int Get_Wave()
	{
		return m_current_round;
	}

	public override string Get_Scoreboard_Value(NetUser user, int column)
	{
		switch (column)
		{
		case 0: return user.Coins;
		case 1: return user.Kills;
		case 2: return user.Deaths;
		case 3: return user.Revives;
		}
		return "< unknown index >";
	}
	
	public override int Get_Scoreboard_Sort_Value(NetUser user)
	{
		return user.Kills;
	}

	public override string Get_Scoreboard_Title()
	{
		return "#scoreboard_title_survive";
	}
	
	public override Team[] Get_Teams()
	{
		return 
		{
			new Team("#scoreboard_team_humans", Vec4(0.9, 0.0, 0.0, 1.0))
		};
	}

	public override Camera Create_Camera(int user_index)
	{
		return new Zoom_Fit_Camera();
	}
	
	public override HUD Create_HUD()
	{
		return new Game_HUD();
	}
	
	public override int Get_Scoreboard_Team_Score(int team_index)
	{
		return 0;
	}

	// -------------------------------------------------------------------------------------------------
	// Calculates current round modifiers.
	// -------------------------------------------------------------------------------------------------
	void Increase_Round()
	{
		m_current_round++;
		
        Player_Manager.Reset();

		Enemy_Manager.Reset();
		Difficulty_Manager.Set_Difficulty(m_current_round);
		Enemy_Manager.Use_Min_Difficulty_Scalars = true;

		// Calculate enemies for this round.
		m_half_time_countdown 		= Math.Max(Half_Time_Minimum_Duration, Half_Time_Duration * Difficulty_Manager.Refactory_Time_Multiplier);	
		m_zombie_spawns_remaining	= Enemies_Per_Round * Difficulty_Manager.Enemy_Spawn_Count_Multiplier;

		On_Round_Change();
	}
	
	// -------------------------------------------------------------------------------------------------
	// Can be derived to extend the wave game.
	// -------------------------------------------------------------------------------------------------
	protected virtual void On_Round_Change()
	{
		// Random weapon plz.
	}
	 
	protected virtual void On_Update_HUD(Game_HUD game_hud, bool new_anim)
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
				Locale.Get("#wave_game_mode_active_countdown").Format(new object[] { m_current_round, m_display_zombies_remaining }),
				m_use_checkoff_animation);
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Pause between waves, does nothing but counts down to the 
	// start of the next wave.
	// -------------------------------------------------------------------------------------------------
	default state State_Refactory_Period
	{
		event void On_Enter()
		{		
			if (Network.Is_Server())
			{
				m_game_state = Wave_Game_State.Refactory;

				if (!m_started)
				{
					if (Cheat_Manager.StartWave > 0)
					{
						m_current_round = Cheat_Manager.StartWave;
					}
				}

				m_started = true;
			}
			
			m_use_checkoff_animation = true;
			m_increased_round = false;
			Freeze_Multiplier();
		}	
		
		event void On_Exit()
		{
			Unfreeze_Multiplier();
		}

		event void On_Tick()
		{	
			if (Network.Is_Server())
			{
				// First run
				if (!m_increased_round)
				{
					Increase_Round();			
					Log.Write("[Game Mode] Half time starting, ");

					m_increased_round = true;
				}

				// Countdown to launch!
				m_half_time_countdown -= Time.Get_Frame_Time();
				if (m_half_time_countdown <= 0)
				{
					if (m_humans_have_spawned)
					{
						change_state(State_Wave_In_Progress);
						return;
					}
				}

				if (!m_humans_have_spawned)
				{
					m_humans_have_spawned = Scene.Find_Actors(typeof(Human_Player)).Length() > 0;
				}
				
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(0);		
				
				// Revive ghosts.
				Player_Manager.Revive_Ghosts();

				// Apply profile updates for users.
				Network.Accept_Pending_Profile_Changes(); 
			}
			else
			{
				if (m_game_state == Wave_Game_State.In_Progress)
				{
					change_state(State_Wave_In_Progress);
				}
				else if (m_game_state == Wave_Game_State.Game_Over)
				{
					change_state(State_Game_Over);
				}
			}

			// Update HUD countdown
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			On_Update_HUD(game_hud, m_use_checkoff_animation);
			m_use_checkoff_animation = false;

			// Tick base code.
			Update_Base();
		}
	}

	event void On_User_Ready(NetUser user)
	{
		user.Team_Index = 0;
	}
	
	// -------------------------------------------------------------------------------------------------
	// Spawns zombies until the allocated amount for this wave is complete.
	// -------------------------------------------------------------------------------------------------
	state State_Wave_In_Progress
	{
		event void On_Enter()
		{ 
			if (Network.Is_Server())
			{
				m_game_state = Wave_Game_State.In_Progress;
			}

			Log.Write("[Wave] Round " + m_current_round);	
			Log.Write("[Wave] Total Spawns This Round: " + m_zombie_spawns_remaining);

			m_use_checkoff_animation = true;

			Audio.Play2D("sfx_hud_round_start");
		} 

		event void On_Tick()
		{
			if (Network.Is_Server())
			{				
				// Spawn zombies plz
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Fodder,		Spawn_Fodder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Chaser,		Spawn_Chaser);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Exploder,	Spawn_Exploder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Bouncer,	Spawn_Bouncer);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Small_Blobby,	Spawn_Small_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Large_Blobby,	Spawn_Large_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Floater,	Spawn_Floater);
				Enemy_Manager.Set_Spawn_Count_Allowed(m_zombie_spawns_remaining);
				Enemy_Manager.Spawning_Enabled = true;

				int enemies_spawned = Enemy_Manager.Spawn_Enemies();
				m_zombie_spawns_remaining -= enemies_spawned;
				
				int enemies_despawned = Enemy_Manager.Despawn_Enemies(true);
				m_zombie_spawns_remaining += enemies_despawned;
				
				if (enemies_spawned > 0)
				{					
					Log.Write("[Wave] Spawned " + enemies_spawned + " enemies, " + m_zombie_spawns_remaining + " remaining.");
				}

				// All zombies dead? Back to refactory plz.
				if (m_zombie_spawns_remaining <= 0 && Enemy_Manager.Count_Wave_Alive() <= 0)
				{
					Audio.Play2D("sfx_hud_round_end");
					change_state(State_Refactory_Period);
				}
				
				// All players dead? Game over time ;_;.
				int alive_player_count = 0;

				Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
				foreach (Human_Player other_player in potential_revivers)
				{
					if (!other_player.Is_Incapacitated())
					{
						alive_player_count++;
					}
				}

				if (alive_player_count <= 0)
				{
					// Kill all incapacitated players off.
					foreach (Human_Player other_player in potential_revivers)
					{
						other_player.Die(null, null);
					}

					change_state(State_Game_Over);
				}
			}
			else
			{
				if (m_game_state == Wave_Game_State.Refactory)
				{
					Audio.Play2D("sfx_hud_round_end");
					change_state(State_Refactory_Period);
				}
				else if (m_game_state == Wave_Game_State.Game_Over)
				{
					change_state(State_Game_Over);
				}
			}

			// Update HUD countdown.
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			On_Update_HUD(game_hud, m_use_checkoff_animation);
			m_use_checkoff_animation = false;

			// Tick base code.
			Update_Base();
		}	
	}
 
	// -------------------------------------------------------------------------------------------------
	// Game over f00l.
	// -------------------------------------------------------------------------------------------------	
	state State_Game_Over
	{
		event void On_Enter()
		{
			if (Network.Is_Server())
			{
				m_game_state = Wave_Game_State.Game_Over;
				Log.Write("[Wave] Game Over!");	
			}
			
			Reset_Game_Over();

			if (!Network.Is_Dedicated_Server())
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_lose");

				Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_base"), Score);
				
				if (!No_Bonuses)
				{
					if (Highest_Multiplier >= 2.0f)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_multiplier"), (<int>(Highest_Multiplier - 1.0f)) * Multiplier_Bonus);
					}
					if (m_current_round > 1)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_round"), (m_current_round - 1) * Round_Bonus);
					}
					if (Network.Get_Primary_Local_User().Kills > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_kills"), Network.Get_Primary_Local_User().Kills * Kill_Bonus);
					}
					if (Network.Get_Primary_Local_User().Revives > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_revives"), Math.Min(Network.Get_Primary_Local_User().Revives, Revive_Bonus_Max_Count) * Revive_Bonus);
					}
				}
			}

			Show_Game_Over();
		}

		event void On_Tick()
		{
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
		m_zombie_spawns_remaining
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_half_time_countdown
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_display_half_time_countdown
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_display_zombies_remaining
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_game_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_current_round
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}