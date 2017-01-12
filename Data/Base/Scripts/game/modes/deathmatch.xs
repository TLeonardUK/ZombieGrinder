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

public enum Deathmatch_Title_Status
{
	None,
	Tied,
	Winning,
	Oneshot
}

public enum Deathmatch_Game_State
{
	Waiting_For_Players,
	In_Progress,
	Game_Over
}

[
	Placeable(true), 
	Name("Deathmatch"), 
	Description("Must kill all humans!") 
]
public class Deathmatch_Game_Mode : Base_Game_Mode
{
	const float WEAPON_DROP_PROBABILITY_INCREASE = 5.0f;
	const int MAX_SCORE = 30;

	const float TITLE_STATUS_DISABLE_DURATION = 5.0f;

	private Deathmatch_Title_Status m_title_status = Deathmatch_Title_Status.None;
	private Deathmatch_Game_State m_game_state = Deathmatch_Game_State.Waiting_For_Players;

	private int m_score = 0;
	private string m_oneshot_title_status = "";
	private float m_title_status_disable_timer = 0.0f;

	private bool m_shown_waiting_for_players = false;


	// -------------------------------------------------------------------------------------------------
	// Construct that shit.
	// -------------------------------------------------------------------------------------------------
	Deathmatch_Game_Mode()
	{
		Gem_Drops = true;
		Weapon_Drops = true;

		Gem_Drops_From_Players = false;
		Weapon_Drops_From_Players = true;
        
        Player_Manager.Reset();
		
		Enemy_Manager.Reset();
		Enemy_Manager.Use_Min_Difficulty_Scalars = false;

		m_weapon_drop_table.Drop_Probability *= WEAPON_DROP_PROBABILITY_INCREASE;
	}

	// -------------------------------------------------------------------------------------------------
	// Derived properties. Used by native code to interact with the script.
	// -------------------------------------------------------------------------------------------------
	public override bool Is_PVP()
	{
		return true;
	}
	
	public override string[] Get_Scoreboard_Columns()
	{
		return 
		{ 
			"#scoreboard_column_kills", 
			"#scoreboard_column_deaths"
		};
	}
	
	public override string Get_Scoreboard_Value(NetUser user, int column)
	{
		switch (column)
		{
		case 0: return user.Kills;
		case 1: return user.Deaths;
		}
		return "< unknown index >";
	}

	public override int Get_Scoreboard_Sort_Value(NetUser user)
	{
		return user.Kills;
	}

	public override string Get_Scoreboard_Title()
	{
		return "#scoreboard_title_kill_everything";
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
		return new Zoom_Fit_Camera(user_index);
	}
	
	public override HUD Create_HUD()
	{
		return new Game_HUD();
	}
	
	public override bool Can_Incapacitate(Pawn other)
	{
		return false;
	}

	public override int Get_Scoreboard_Team_Score(int team_index)
	{
		return 0;
	}

	// -------------------------------------------------------------------------------------------------
	// In-progress state!
	// -------------------------------------------------------------------------------------------------
	default state State_Waiting_For_Players
	{
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				// Enough players?
				if (Network.Active_Team_Count() >= 2)
				{
					Log.Write("[GM] Starting game, we have enough players.");
					m_game_state = Deathmatch_Game_State.In_Progress;
				}
			}

			if (m_game_state == Deathmatch_Game_State.In_Progress)
			{
				change_state(State_In_Progress);
				return;
			}
			else if (m_game_state == Deathmatch_Game_State.Game_Over)
			{
				change_state(State_Game_Over);
				return;
			}

			Game_HUD game_hud = <Game_HUD>Get_HUD();
			game_hud.Set_Objective_Text(Locale.Get("#pvp_game_mode_waiting_for_players"), !m_shown_waiting_for_players, true);
			m_shown_waiting_for_players = true;
			
			// Tick base code.
			Update_Base();
		}
	}
	
	state State_In_Progress
	{
		event void On_Tick()
		{
			// Game over?
			if (Network.Is_Server())
			{
				if (Network.Active_Team_Count() < 2)
				{
					Log.Write("[GM] Starting game over due to too few players.");
					m_game_state = Deathmatch_Game_State.Game_Over;
				}
			}
			if (m_game_state == Deathmatch_Game_State.Game_Over)
			{
				change_state(State_Game_Over);
				return;
			}
			
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(-1, true, true);		
			}

			// Update HUD countdown.
			Update_Title_Status();
			
			// Tick base code.
			Update_Base();
		}	
	}

	void Update_Title_Status()
	{
		float delta_t = Time.Get_Delta_Seconds();
		Game_HUD game_hud = <Game_HUD>Get_HUD();
		NetUser[] users = Network.Get_Users();

		Deathmatch_Title_Status old_title_status = m_title_status;
		string title_text = "";

		m_title_status_disable_timer -= delta_t;
		
		// Find the best user.
		NetUser best_user = null;
		foreach (NetUser u in users)
		{
			if ((best_user != null && u.Kills > best_user.Kills) || (best_user == null && u.Kills > 0))
			{
				best_user = u;
			}
		}

		// If null we are tied.
		if (best_user == null)
		{
			title_text = Locale.Get("#dm_game_mode_tie_status");
			m_title_status = Deathmatch_Title_Status.Tied;
		}

		// Otherwise the dickbutt is winning.
		else
		{
			title_text = Locale.Get("#dm_game_mode_winning_status").Format(new object[] { best_user.Username, best_user.Kills });
			m_title_status = Deathmatch_Title_Status.Winning;
		}
			
		// Set the status!
		if (m_title_status_disable_timer <= 0.0f)
		{
			// Oneshot!
			if (m_oneshot_title_status != "")
			{
				title_text = m_oneshot_title_status;
				m_oneshot_title_status = "";
				m_title_status = Deathmatch_Title_Status.Oneshot;
				m_title_status_disable_timer = TITLE_STATUS_DISABLE_DURATION;
			
				game_hud.Set_Objective_Text(title_text, true);
			}
			else
			{
				game_hud.Set_Objective_Text(title_text, old_title_status != m_title_status);
			}
		}
	}

	event void On_User_Ready(NetUser user)
	{
		NetUser[] users = Network.Get_Users();

		// Assing user to the next empty team index.
		int highest_index = 0;
		foreach (NetUser u in users)
		{
			if (u != user)
			{
				if (u.Team_Index >= highest_index)
				{
					highest_index = u.Team_Index + 1;
				}
			}
		}
		user.Team_Index = highest_index;

		Log.Write("Assigning user '"+user.Username+"' to team "+highest_index);
	}
 
	// -------------------------------------------------------------------------------------------------
	//	Damage handling.
	// -------------------------------------------------------------------------------------------------
	override event void On_Pawn_Killed(Event_Pawn_Killed evt)
	{		
		base.On_Pawn_Killed(evt);
		m_score++;

		int remaining = MAX_SCORE - m_score;
		if (remaining < 5 && remaining > 0)
		{
			if (remaining == 1)
			{
				Audio.Play2D("sfx_hud_round_start");
				m_oneshot_title_status = Locale.Get("#dm_game_mode_final_kill_status");
			}
			else
			{
				m_oneshot_title_status = Locale.Get("#dm_game_mode_kills_left_status").Format(new object[] { remaining });
			}			
		}

		if (remaining == 0)
		{
			if (Network.Is_Server())
			{
				Log.Write("[DM] Score limit reached, game over!");
				m_game_state = Deathmatch_Game_State.Game_Over;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Game over f00l.
	// -------------------------------------------------------------------------------------------------	
	state State_Game_Over
	{
		event void On_Enter()
		{
			Audio.Play2D("sfx_hud_round_end");

			if (Network.Is_Server())
			{				
				Player_Manager.Kill_Humans();					
			}

			Reset_Game_Over();
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
		m_score
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_game_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}