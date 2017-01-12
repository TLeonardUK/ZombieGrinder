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

public enum Taem_Deathmatch_Title_Status
{
	None,
	Tied,
	Winning,
	Oneshot
}

public enum Team_Deathmatch_Game_State
{
	Waiting_For_Players,
	In_Progress,
	Game_Over
}

public class Team_Deathmatch_Team_State
{
	public int Score;
}

[
	Placeable(true), 
	Name("Team Deathmatch"), 
	Description("Must kill all humans!") 
]
public class Taem_Deathmatch_Game_Mode : Base_Game_Mode
{
	const int MAX_SCORE = 30;

	const float WEAPON_DROP_PROBABILITY_INCREASE = 5.0f;
	const float TITLE_STATUS_DISABLE_DURATION = 5.0f;

	private Taem_Deathmatch_Title_Status m_title_status = Taem_Deathmatch_Title_Status.None;
	private Team_Deathmatch_Game_State m_game_state = Team_Deathmatch_Game_State.Waiting_For_Players;

	private int m_score = 0;
	private string m_oneshot_title_status = "";
	private float m_title_status_disable_timer = 0.0f;

	private Team[] m_teams;
	private Team_Deathmatch_Team_State[] m_team_state;

	private bool m_shown_waiting_for_players = false;

	// -------------------------------------------------------------------------------------------------
	// Construct that shit.
	// -------------------------------------------------------------------------------------------------
	Taem_Deathmatch_Game_Mode()
	{
		Gem_Drops = true;
		Weapon_Drops = true;

		Gem_Drops_From_Players = false;
		Weapon_Drops_From_Players = true;
        
        Player_Manager.Reset();

		Enemy_Manager.Reset();
		Enemy_Manager.Use_Min_Difficulty_Scalars = false;

		m_weapon_drop_table.Drop_Probability *= WEAPON_DROP_PROBABILITY_INCREASE; 
		
		m_teams = new Team[]
		{
			new Team("#scoreboard_team_red", Vec4(0.9, 0.4, 0.4, 1.0)),
			new Team("#scoreboard_team_blue", Vec4(0.4, 0.4, 0.9, 1.0))
		};
		
		m_team_state = new Team_Deathmatch_Team_State[m_teams.Length()];
		
		for (int i = 0; i < m_teams.Length(); i++)
		{
			m_team_state[i] = new Team_Deathmatch_Team_State();
		}
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
		return m_teams;
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
	
	public override bool Should_Tint_Teams()
	{
		return true;
	}
	
	public override int Get_Scoreboard_Team_Score(int team_index)
	{
		return m_team_state[team_index].Score;
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
					m_game_state = Team_Deathmatch_Game_State.In_Progress;
				}
			}

			if (m_game_state == Team_Deathmatch_Game_State.In_Progress)
			{
				change_state(State_In_Progress);
				return;
			}
			else if (m_game_state == Team_Deathmatch_Game_State.Game_Over)
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
					m_game_state = Team_Deathmatch_Game_State.Game_Over;
				}
			}
			if (m_game_state == Team_Deathmatch_Game_State.Game_Over)
			{
				change_state(State_Game_Over);
				return;
			}

			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(-1, true);		
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

		Taem_Deathmatch_Title_Status old_title_status = m_title_status;
		string title_text = "";

		m_title_status_disable_timer -= delta_t;
		
		// Find the best team.
		int best_team_index = -1;
		for (int i = 0; i < m_teams.Length(); i++)
		{
			if (m_team_state[i].Score > 0)
			{
				if (best_team_index < 0 || m_team_state[i].Score > m_team_state[best_team_index].Score)
				{
					best_team_index = i;
				}
			}
		}

		// If null we are tied.
		if (best_team_index <= -1)
		{
			title_text = Locale.Get("#tdm_game_mode_tie_status");
			m_title_status = Deathmatch_Title_Status.Tied;
		}

		// Otherwise the dickbutt is winning.
		else
		{
			Vec4 team_color = m_teams[best_team_index].Primary_Color * 255.0f;

			title_text = Locale.Get("#tdm_game_mode_winning_status").Format(new object[] { <int>team_color.X, <int>team_color.Y, <int>team_color.Z, Locale.Get(m_teams[best_team_index].Name), m_team_state[best_team_index].Score });
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

		// Find team with fewest players and shunt the user onto it.
		int[] team_counts = new int[m_teams.Length()];

		foreach (NetUser u in users)
		{
			if (u != user)
			{
				if (u.Team_Index >= 0 && u.Team_Index < m_teams.Length())
				{
					team_counts[u.Team_Index]++;
				}
			}
		}
		
		// Find lowet team count.
		int best_team_index = -1;
		int team_index_count = 0;
		for (int i = 0; i < m_teams.Length(); i++)
		{
			if (team_counts[i] < team_index_count || best_team_index < 0)
			{
				best_team_index = i;
				team_index_count = team_counts[i];
			}
		}

		user.Team_Index = best_team_index;
		Log.Write("Assigning user '"+user.Username+"' to team "+best_team_index);
	}
 
	// -------------------------------------------------------------------------------------------------
	//	Damage handling.
	// -------------------------------------------------------------------------------------------------
	override event void On_Pawn_Killed(Event_Pawn_Killed evt)
	{			
		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Killer;
		NetUser killer_user  = killer_actor == null ? null : killer_actor.Owner;

		if (killer_actor != null)
		{
			NetUser imposer = killer_actor.Get_Imposing_As();
			if (imposer != null)
			{
				killer_user = imposer;			
				killer_actor = null;

				if (killer_user.Controller != null)
				{
					if (killer_user.Controller.Possessed != null)
					{
						killer_actor = killer_user.Controller.Possessed;
					}
				}
			}
		}

		base.On_Pawn_Killed(evt);

		// Increment global count.
		m_score++;

		// Add score to team.
		if (killer_user != null)
		{
			m_team_state[killer_user.Team_Index].Score++;
		}

		int remaining = MAX_SCORE - m_score;
		if (remaining < 5 && remaining > 0)
		{
			if (remaining == 1)
			{
				Audio.Play2D("sfx_hud_round_start");
				m_oneshot_title_status = Locale.Get("#tdm_game_mode_final_kill_status");
			}
			else
			{
				m_oneshot_title_status = Locale.Get("#tdm_game_mode_kills_left_status").Format(new object[] { remaining });
			}			
		}

		if (remaining == 0)
		{
			if (Network.Is_Server())
			{
				Log.Write("[DM] Score limit reached, game over!");
				m_game_state = Team_Deathmatch_Game_State.Game_Over;
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
		m_team_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
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