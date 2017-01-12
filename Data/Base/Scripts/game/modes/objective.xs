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

[
	Placeable(true), 
	Name("Objective"), 
	Description("Objective to show to a user in the Objective Game Mode. First enabled one found is considered the current one.") 
]
public class Objective : Actor
{
	[ 
		Name("Text"), 
		Description("Text to show in HUD for objective.")
	]
	public serialized(1) string Text = "< Not Set >";
	
	Objective()
	{
		Enabled = false;
		Bounding_Box = Vec4(0, 0, 32, 32);
	}
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		this.Enabled = !this.Enabled;
		Disable_Others();

		Log.Write("Objective '"+this.Tag+"' was activated.");
	}

	// Disables other objectives.
	public void Disable_Others()
	{
		Actor[] potential = Scene.Find_Actors(typeof(Objective));
		foreach (Objective o in potential)
		{
			if (o != this)
			{
				o.Enabled = false;
			}
		}
	}
	
	// Sprite to show in the editor.
	components
	{
		SpriteComponent sprite
		{
			Frame 			= "actor_objective_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
		Enabled
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}

[
	Placeable(true), 
	Name("Timed Hold Out Objective"), 
	Description("Hold out for the given time.") 
]
public class Timed_Hold_Out_Objective : Objective
{
	[ 
		Name("Time"), 
		Description("Time to hold out for (in seconds).")
	]
	public serialized(1) float Duration = 2 * 60.0f;
	
	// -------------------------------------------------------------------------------------------------
	// Gameplay modifiers.
	// -------------------------------------------------------------------------------------------------
	public serialized(220) bool Spawn_Fodder		= true;
	public serialized(220) bool Spawn_Chaser		= true;
	public serialized(220) bool Spawn_Exploder		= true;
	public serialized(220) bool Spawn_Bouncer		= false;
	public serialized(220) bool Spawn_Small_Blobby	= false;
	public serialized(220) bool Spawn_Large_Blobby	= false;
	public serialized(700) bool Spawn_Floater		= false;

	private bool m_triggered_end = false;
	private float m_time_left = 0.0f;

	Timed_Hold_Out_Objective()
	{
		Enabled = false;
		Bounding_Box = Vec4(0, 0, 32, 32);
	}
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		this.Enabled = !this.Enabled;

		if (this.Enabled)
		{
			m_time_left = Duration;
			m_triggered_end = false;
		}
		
		Disable_Others();
	}
	
	default state State_Idle
	{
		public event void On_Tick()
		{
			if (Network.Is_Server())
			{ 
				m_time_left -= Time.Get_Delta_Seconds();
				 
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Fodder,			Spawn_Fodder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Chaser,			Spawn_Chaser);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Exploder,		Spawn_Exploder);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Bouncer,		Spawn_Bouncer);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Small_Blobby,	Spawn_Small_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Large_Blobby,	Spawn_Large_Blobby);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Floater,		Spawn_Floater);
			}

			int time = <int>m_time_left;
			int minutes = time / 60;
			int seconds = (time % 60);  
			string timer = (<string>minutes).PadLeft(2, "0") + ":" + (<string>seconds).PadLeft(2, "0");
			Text = Locale.Get("#objective_hold_out").Format(new object[] { timer });
		
			if (m_time_left <= 0.0f && m_triggered_end == false)
			{	
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Fodder,			true);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Chaser,			false);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Exploder,		false);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Bouncer,		false);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Small_Blobby,	false);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Large_Blobby,	false);
				Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Floater,		false);

				rpc(RPCTarget.All, RPCTarget.None) RPC_Trigger_End();

				m_triggered_end = true;
			}
		}
	}
	
	private rpc void RPC_Trigger_End()
	{
		Log.Write("Hold out finished, triggering " + Link);
		Scene.Trigger_Actors(Link);
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
		m_time_left
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}

[
	Placeable(true), 
	Name("Level End Objective"), 
	Description("When enabled the level is over.") 
]
public class Level_End_Objective : Objective
{
	[ 
		Name("Destination"), 
		Description("Destination to go to after level-end, either a level name or 'credits' to finish a campaign. If empty map will restart.")
	]
	public serialized(250) string Destination = "";
	
	[ 
		Name("Outro Point Link"), 
		Description("If set the player will move from its current position to the outro position on end.") 
	]
	public serialized(3500) string Outro_Point_Link;
	
    float m_game_over_timer = 0.0f;

    bool m_outro_started = false;

	Level_End_Objective()
	{
		Enabled = false;
		Bounding_Box = Vec4(0, 0, 32, 32);
        Text = "";
		Outro_Point_Link = "";
        m_outro_started = false;
	}

	default state State_Idle
	{
		public event void On_Tick()
		{
			Objective_Game_Mode mode = <Objective_Game_Mode>Scene.Active_Game_Mode;
		
			if (Network.Is_Server())
			{ 			
				if (m_outro_started)
				{
					Actor[] players = Scene.Find_Actors(typeof(Player_Human_Controller));

					bool bIsFinished = true;
					
					foreach (Player_Human_Controller player in players)
					{
						if (!player.Is_Outro_Finished())
						{
							bIsFinished = false;
							break;
						}
					}
					
					if (bIsFinished)
					{
                        Log.Write("Outro for " + players.Length() + " players has finished, starting game over");
						Game_Over();
						m_outro_started = false;
					}
				}	
				
                if (m_game_over_timer > 0.0f)
                {
                    m_game_over_timer -= Time.Get_Delta_Seconds();
                    if (m_game_over_timer <= 0.0f)
                    {
						if (Outro_Point_Link != "")
						{
							if (!m_outro_started)
							{
								m_outro_started = true;			
								Enemy_Manager.Despawn_All_Enemies();	

					            Actor[] points = Scene.Find_Actors_By_Tag(Outro_Point_Link);
					            if (points.Length() > 0)
					            {
						            Player_Spawn_Cutscene_Point outro_point = <Player_Spawn_Cutscene_Point>points[0];
						
					                Actor[] players = Scene.Find_Actors(typeof(Player_Human_Controller));
					                foreach (Player_Human_Controller player in players)
					                {
						                 player.Start_Outro(outro_point);
                                    }

                                    mode.m_outro_played = true;
					            }

								mode.bInOutro = true;
							}
						}
						else
						{
                            Log.Write("Level end objective active, starting game over.");
							Game_Over();
						}
                    }
                }
            }		
        }
    }   
	
	void Game_Over()
	{	
		Objective_Game_Mode mode = <Objective_Game_Mode>Scene.Active_Game_Mode;
		if (mode != null)
		{
			mode.Game_Over(true, Destination);
		}
	}
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		this.Enabled = !this.Enabled;
		Disable_Others();
        
        Text = "";
        m_game_over_timer = 1.0f;
	}
}

public enum Objective_Game_State
{
	In_Progress		= 0,
	Game_Over		= 2
}

[
	Placeable(true), 
	Name("Objective Mode"), 
	Description("Shows the user an objective on the HUD, thats about it.") 
]
public class Objective_Game_Mode : Base_Game_Mode
{
	private Objective m_current_objective;
	private string	  m_current_objective_text;
	
	// Maximum number of seconds used for calculating time bonus.
	private const float Max_Time = 15.0f * 60.0f;
	private const float Time_Bonus_Per_Second = 100.0f;

	private serialized(160) bool m_level_complete = false;
	private serialized(200) string m_level_complete_destination = "";
	private serialized(161) bool m_some_have_been_alive = false;

    public serialized(3000) bool bLimitEveryone = false;

	public serialized(3500) bool bInOutro = false;

    public bool m_outro_played = false;
	
	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------
	protected Objective_Game_State m_game_state = Objective_Game_State.In_Progress;

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
	
	public override string Get_Scoreboard_Title()
	{
		return "#scoreboard_title_follow_objectives";
	}
	
	public override int Get_Scoreboard_Sort_Value(NetUser user)
	{
		return user.Kills;
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
		return new Zoom_Fit_Camera(-1, bLimitEveryone);
	}
	
	public override HUD Create_HUD()
	{
		return new Game_HUD();
	}
	
	public override int Get_Scoreboard_Team_Score(int team_index)
	{
		return 0;
	}
	
	Objective_Game_Mode()
	{
		Enemy_Manager.Reset();
        Player_Manager.Reset();
	}

	// -------------------------------------------------------------------------------------------------
	// Just showing objectives!
	// -------------------------------------------------------------------------------------------------
	default state State_Idle
	{
		event void On_Enter()
		{		
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Fodder,			true);
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Chaser,			true);
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Exploder,		true);
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Large_Blobby,	true);
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Small_Blobby,	true);
			Enemy_Manager.Set_Enemy_Type_Enabled(Enemy_Type.Floater,		true);
			Enemy_Manager.Allow_Path_Spawns = true;
			Enemy_Manager.Spawning_Enabled = false;

			Enemy_Manager.Use_Min_Difficulty_Scalars = false;
            Difficulty_Manager.Use_Fear_Difficulty = true;
			Difficulty_Manager.Set_Difficulty(3.0f);
            
            m_some_have_been_alive = false;
            bInOutro = false;
            bLimitEveryone = false;

			// Started in game-over?
			if (!Network.Is_Server())
			{
				if (m_game_state == Objective_Game_State.Game_Over)
				{
					change_state(State_Game_Over);
				}
			}
		}

		event void On_Tick()
		{	
			// Find current objective.
			bool new_objective = false;

			Actor[] potential = Scene.Find_Actors(typeof(Objective));
			foreach (Objective o in potential)
			{
				if (o.Enabled)
				{
					if (o != m_current_objective)
					{
						m_current_objective_text = Locale.Get(o.Text);
						m_current_objective = o;

						new_objective = true;
					}
				}
			}

			if (m_current_objective != null)
			{
				m_current_objective_text = Locale.Get(m_current_objective.Text);
			}

			// Update HUD countdown.
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			game_hud.Set_Objective_Text(
				m_current_objective_text,
				new_objective);

			// Compass pip.
			if (m_current_objective != null)
			{				
				Compass.Add_Pip(m_current_objective.Position, "game_hud_compass_objective", -1, Vec4(255.0f, 255.0f, 255.0f, 255.0f));
			}

			// Change state?	
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				if (!bInOutro)
				{
					Player_Manager.Spawn_Humans(0, false, false, true, true, false);	
					
					// Spawn enemies.			
					Enemy_Manager.Spawn_Enemies();				
					Enemy_Manager.Despawn_Enemies();	

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
						if (m_some_have_been_alive == true)
						{
                            Log.Write("Starting game over due to humans having been alive but none alive any longer :(");
                            Game_Over(false, "");
						}
					}
					else
					{
						m_some_have_been_alive = true;
					}
    			}
			}

			// Tick base code.
			Update_Base();
		}
	}

	event void On_User_Ready(NetUser user)
	{
		user.Team_Index = 0;
	}
	
	// -------------------------------------------------------------------------------------------------	
	// Game over f00l.
	// -------------------------------------------------------------------------------------------------	
	public void Game_Over(bool level_complete, string destination)
	{
		if (Network.Is_Server())
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Game_Over(level_complete, destination);
		}
	}

	public rpc void RPC_Game_Over(bool level_complete, string destination)
	{
		Log.Write("Game Over Invoked, destination="+destination);

		m_level_complete = level_complete;
		m_level_complete_destination = destination;
		change_state(State_Game_Over);
	}

	state State_Game_Over
	{
		event void On_Enter()
		{
			if (Network.Is_Server())
			{				
				// Kill all incapacitated players off.
				Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));

				foreach (Human_Player other_player in potential_revivers)
				{
                    if (m_outro_played)
                    {
                        other_player.Despawn();
                    }
                    else
                    {
					    other_player.SuccessDie();
                    }
				}

				m_game_state = Objective_Game_State.Game_Over;
				Log.Write("[Wave] Game Over!");	
			}

			if (m_level_complete)
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_win");
			}
			else
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_lose");
			}

			float time_bonus = Math.Max(0.0f, Max_Time - Game_Time);

			Reset_Game_Over();

			if (!Network.Is_Dedicated_Server())
			{
				if (m_level_complete)
				{
					Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_base"), Score);

					if (Highest_Multiplier >= 2.0f)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_multiplier"), (<int>(Highest_Multiplier - 1.0f)) * Multiplier_Bonus);
					}
					if (time_bonus > 1)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_time"), time_bonus * Time_Bonus_Per_Second);
					}
					if (Network.Get_Primary_Local_User().Kills > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_kills"), Math.Min(Max_Kill_Bonus, Network.Get_Primary_Local_User().Kills * Kill_Bonus));
					}
					if (Network.Get_Primary_Local_User().Revives > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_revives"), Math.Min(Network.Get_Primary_Local_User().Revives, Revive_Bonus_Max_Count) * Revive_Bonus);
					}
				}
			}

			Show_Game_Over(m_level_complete ? Locale.Get("#game_over_level_complete_title") : "", m_level_complete_destination);
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{	
		m_game_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}