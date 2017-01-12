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

public enum CTB_Title_Status
{
	None,
	Start_Countdown,
	Respawn_Countdown,
	Carry_Enemy,
	Carry_Team,
	Capture,
	Oneshot
}

public enum CTB_Game_State
{
	Waiting_For_Players,
	In_Progress,
	Game_Over
}

public class CTB_Team_State
{
	public int Score;
}

[
	Placeable(true), 
	Name("Capture The Bag"), 
	Description("Must kill all humans!") 
]
public class CTB_Game_Mode : Base_Game_Mode
{
	const int MAX_SCORE = 5;

	const float TITLE_STATUS_DISABLE_DURATION = 3.0f;
	const float BAG_RESPAWN_DELAY = 20.0f;	
	const float MATCH_START_DELAY = 10.0f;

	private float m_match_start_timer = 0.0f;
	private float m_bag_respawn_timer = 0.0f;

	private CTB_Title_Status m_title_status = CTB_Title_Status.None;
	private CTB_Game_State m_game_state = CTB_Game_State.Waiting_For_Players;

	private string m_oneshot_title_status = "";
	private float m_title_status_disable_timer = 0.0f;

	private Team[] m_teams;
	private CTB_Team_State[] m_team_state;

	private bool m_has_show_final_capture_warning = false;
	
	private Vec3 m_original_bag_position;
	private bool m_got_original_bag_position = false;

	private bool m_shown_waiting_for_players = false;

	// -------------------------------------------------------------------------------------------------
	// Construct that shit.
	// -------------------------------------------------------------------------------------------------
	CTB_Game_Mode()
	{
		Gem_Drops = true;
		Weapon_Drops = true;

		Gem_Drops_From_Players = false;
		Weapon_Drops_From_Players = false;
        
        Player_Manager.Reset();

		Enemy_Manager.Reset();
		Enemy_Manager.Use_Min_Difficulty_Scalars = false;

		m_teams = new Team[]
		{
			new Team("#scoreboard_team_red", Vec4(0.9, 0.4, 0.4, 1.0)),
			new Team("#scoreboard_team_blue", Vec4(0.4, 0.4, 0.9, 1.0)),
			new Team("#scoreboard_team_green", Vec4(0.4, 0.9, 0.4, 1.0)),
			new Team("#scoreboard_team_yellow", Vec4(0.9, 0.9, 0.4, 1.0))
		};
		
		m_team_state = new CTB_Team_State[m_teams.Length()];
		
		for (int i = 0; i < m_teams.Length(); i++)
		{
			m_team_state[i] = new CTB_Team_State();
		}

		m_match_start_timer = MATCH_START_DELAY;
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
			"#scoreboard_column_captures", 
			"#scoreboard_column_kills", 
			"#scoreboard_column_deaths"
		};
	}
	
	public override string Get_Scoreboard_Value(NetUser user, int column)
	{
		switch (column)
		{
		case 0: return user.Score;
		case 1: return user.Kills;
		case 2: return user.Deaths;
		}
		return "< unknown index >";
	}

	public override int Get_Scoreboard_Sort_Value(NetUser user)
	{
		return user.Score;
	}

	public override string Get_Scoreboard_Title()
	{
		return "#scoreboard_title_capture_the_bag";
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
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(-1, true);		
			
				// Enough players?
				if (Network.Active_Team_Count() >= 2)
				{
					Log.Write("[GM] Starting game, we have enough players.");
					m_game_state = CTB_Game_State.In_Progress;
				}
			}

			if (m_game_state == CTB_Game_State.In_Progress)
			{
				change_state(State_In_Progress);
				return;
			}
			else if (m_game_state == CTB_Game_State.Game_Over)
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
					m_game_state = CTB_Game_State.Game_Over;
				}
			}
			if (m_game_state == CTB_Game_State.Game_Over)
			{
				change_state(State_Game_Over);
				return;
			}
			
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(-1, true);	
			}

			// Update match start.
			if (Network.Is_Server())
			{
				if (m_got_original_bag_position == false)
				{		
					Bag_Pickup bag = Get_Bag_Pickup();
					if (bag != null)
					{
						m_original_bag_position = bag.Position;
						m_got_original_bag_position = true;
						
						bag.Despawn();
					}
				}

				if (m_match_start_timer > 0.0f)
				{
					m_match_start_timer -= Time.Get_Delta_Seconds();
					if (m_match_start_timer <= 0.0f)
					{
						Log.Write("[CTB] Match Start!");		
						
						Scene.Trigger_Actors("Match_Start");
						
						m_bag_respawn_timer = BAG_RESPAWN_DELAY;
					}
				}
			
				// Update bag respawn.
				if (m_bag_respawn_timer > 0.0f)
				{
					m_bag_respawn_timer -= Time.Get_Delta_Seconds();
					if (m_bag_respawn_timer <= 0.0f)
					{
						Log.Write("[CTB] Bag Spawn!");
						
						Bag_Pickup bag = Get_Bag_Pickup();
						if (bag != null) // If this is null we are kinda fucked...
						{
							bag.Position = m_original_bag_position;
							bag.Force_Spawn();
						}
					}
				}
			}

			// Update HUD countdown.
			Update_Title_Status();
			
			// Tick base code.
			Update_Base();
		}	
	}

	Bag_Pickup Get_Bag_Pickup()
	{
		Actor[] actors = Scene.Find_Actors(typeof(Bag_Pickup));
		if (actors.Length() > 0)
		{
			return <Bag_Pickup>actors[0];
		}
		else
		{
			return null;
		}
	}

	void Update_Title_Status()
	{
		float delta_t = Time.Get_Delta_Seconds();
		Game_HUD game_hud = <Game_HUD>Get_HUD();
		NetUser[] users = Network.Get_Users();

		CTB_Title_Status old_title_status = m_title_status;
		string title_text = "";

		m_title_status_disable_timer -= delta_t;

		// Grab the bag.
		Bag_Pickup bag = Get_Bag_Pickup();
		
		// If split screen simplify messages.
		if (bag != null)
		{
			Actor pickup_pawn = Scene.Get_Net_ID_Actor(bag.Player_Owner_ID);
			if (!bag.Is_Spawned && pickup_pawn != null && pickup_pawn.Owner != null)
			{
				if (Scene.Get_Camera_Count() > 1)
				{
					// Show "X has the bag!"
					Vec4 team_color = m_teams[pickup_pawn.Owner.Team_Index].Primary_Color * 255;

					title_text = Locale.Get("#ctb_game_mode_bag_retrieve_status").Format(new object[] { 
						<int>team_color.X, <int>team_color.Y, <int>team_color.Z, 
						pickup_pawn.Owner.Display_Username
					});	

					m_title_status = CTB_Title_Status.Carry_Enemy;		
				}
				else
				{
					NetUser primary_user = Network.Get_Primary_Local_User();

					// Show "Kill The Bag Carrier!"
					if (primary_user != null && primary_user.Team_Index != pickup_pawn.Owner.Team_Index)
					{
						title_text = Locale.Get("#ctb_game_mode_bag_carry_status");
						m_title_status = CTB_Title_Status.Carry_Enemy;		
					}
		
					// Show "Protect The Bag Carrier!"
					else
					{
						title_text = Locale.Get("#ctb_game_mode_bag_carry_team_status");
						m_title_status = CTB_Title_Status.Carry_Team;		
					}
				}

				// Add the compass pip for the bag.
				Compass.Add_Pip(pickup_pawn.Position, "game_hud_ctb_bags_" + (pickup_pawn.Owner.Team_Index + 1), pickup_pawn.Owner.Is_Local ? pickup_pawn.Owner.Local_Index : -1, Vec4(255.0f, 255.0f, 255.0f, 255.0f));
			}
			else
			{
				// Show "Capture The Bag!"
				title_text = Locale.Get("#ctb_game_mode_bag_capture_status");
				m_title_status = CTB_Title_Status.Capture;		

				// Add the compass pip for the bag.
				if (bag.Is_Spawned)
				{
					Compass.Add_Pip(bag.Position, "game_hud_ctb_bags_5", -1, Vec4(255.0f, 255.0f, 255.0f, 255.0f));
				}
			}
		}

		// Show "bag respawning in 3..2..1..
		if (m_bag_respawn_timer >= 0.0f)
		{
			title_text = Locale.Get("#ctb_game_mode_bag_respawn_timer").Format(new object[] { <int>m_bag_respawn_timer + 1 });			
			m_title_status = CTB_Title_Status.Respawn_Countdown;		
		}

		// Show match start in 3..2..1..
		if (m_match_start_timer >= 0.0f)
		{
			title_text = Locale.Get("#ctb_game_mode_countdown_status").Format(new object[] { <int>m_match_start_timer + 1 });	
			m_title_status = CTB_Title_Status.Start_Countdown;		
		}

		// Set the status!
		if (m_title_status_disable_timer <= 0.0f)
		{
			// Oneshot!
			if (m_oneshot_title_status != "")
			{
				title_text = m_oneshot_title_status;
				m_oneshot_title_status = "";
				m_title_status = CTB_Title_Status.Oneshot;
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
		base.On_Pawn_Killed(evt);
	}
	
	// -------------------------------------------------------------------------------------------------
	//	Bag capturing.
	// -------------------------------------------------------------------------------------------------
	public event void Trigger_Capture(Actor capturer)
	{
		if (Network.Is_Server())
		{
			Bag_Pickup pickup = <Bag_Pickup>Inventory_Pickup.Get_Inventory_Item_For_Net_ID(capturer.Net_ID);
			if (pickup != null)
			{		
				bool game_winning_capture = false;
				
				// Increment score.
				capturer.Owner.Score++;
				m_team_state[capturer.Owner.Team_Index].Score++;
				if (m_team_state[capturer.Owner.Team_Index].Score >= MAX_SCORE)
				{
					Log.Write("[CTB] Score limit reached, game over!");
					m_game_state = CTB_Game_State.Game_Over;
					game_winning_capture = true;
				}
				
				// Notify users!
				rpc(RPCTarget.All, RPCTarget.None) RPC_Bag_Captured(capturer.Owner.Team_Index, game_winning_capture);

				// Despawn the bag and start countdown to respawn.
				pickup.Reset();
				pickup.Despawn();
				m_bag_respawn_timer = BAG_RESPAWN_DELAY;
				
				// Captured the bag message.
				Team team = m_teams[capturer.Owner.Team_Index];
				Vec4 team_color = team.Primary_Color * 255.0f;
				Network.Send_Chat(ChatType.Server, Locale.Get("#chat_message_player_captured_bag").Format(new object[] { <int>team_color.X, <int>team_color.Y, <int>team_color.Z, Locale.Get(team.Name) }));
			
				Log.Write(capturer.Net_ID+" captured a bag, omg!");
			}
		}
	}
	
	private rpc void RPC_Bag_Captured(int team_index, bool game_winning_capture)
	{
		// Play capture/loss sound.
		NetUser[] users = Network.Get_Local_Users();
		bool is_local_team = false;

		foreach (NetUser user in users)
		{
			if (user.Team_Index == team_index)
			{
				is_local_team = true;
			}
		}
		 
		if (is_local_team == true)
		{
			if (game_winning_capture == false)
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_win");
			}
			else
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_win");
			}
		}
		else
		{
			if (game_winning_capture == false)
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_lose");
			}
			else
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_lose");
			}
		}

		// Show message about capture.
		if (game_winning_capture == false)
		{
			Vec4 team_color = m_teams[team_index].Primary_Color * 255;

			m_oneshot_title_status = Locale.Get("#ctb_game_mode_capture_status").Format(new object[] { 
				<int>team_color.X, <int>team_color.Y, <int>team_color.Z, 
				Locale.Get(m_teams[team_index].Name)
			});						
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Game over f00l.
	// -------------------------------------------------------------------------------------------------	
	state State_Game_Over
	{
		event void On_Enter()
		{
		//	Audio.Play2D("sfx_hud_round_end");

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
		m_game_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_bag_respawn_timer
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_match_start_timer
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}