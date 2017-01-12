using runtime.math;
using runtime.log;
using system.game_mode;
using system.time;
using system.network;
using system.audio;
using game.modes.huds.game_hud;
using game.modes.base_game_mode;
using game.difficulty.difficulty_manager;
using game.actors.player.player;
using game.actors.player.player_manager;
using game.actors.cameras.zoom_fit_camera;
using game.actors.enemies.enemy_manager;

public enum Example_Game_Mode_State
{
	Waiting_For_Players	= 0,
	Countdown			= 1,
	Spawning			= 2,
	Game_Over			= 3
}

[
	Placeable(true), 
	Name("Example Game Mode"), 
	Description("An example of a custom game mode.") 
]
public class Example_Game_Mode : Base_Game_Mode
{ 
	protected float COUNTDOWN_DURATION = 10.0f;

	protected Example_Game_Mode_State m_game_state;

	// -------------------------------------------------------------------------------------------------
	// Properties that should be replicated to other clients.
	// -------------------------------------------------------------------------------------------------
	replicate(ReplicationPriority.Normal)
	{	
		m_game_state
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
	
	// -------------------------------------------------------------------------------------------------
	// Overridden properties of the base game mode. These need to be defined.
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
	
	public override int Get_Scoreboard_Sort_Value(NetUser user)
	{
		return user.Kills;
	}

	public override string Get_Scoreboard_Title()
	{
		return "Kill The Enemies!";
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
	// Invoked when user joins the game. Used to setup game specific state - which team they are on etc.
	// -------------------------------------------------------------------------------------------------
	event void On_User_Ready(NetUser user)
	{
		user.Team_Index = 0;
	}
	
	// -------------------------------------------------------------------------------------------------
	// Waits for players to join game.
	// -------------------------------------------------------------------------------------------------
	default state State_Waiting_For_Players
	{
		event void On_Enter()
		{		
			if (Network.Is_Server())
			{
				m_game_state = Example_Game_Mode_State.Waiting_For_Players;
			}
		}	
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(0);		

				// Revive ghosts.
				Player_Manager.Revive_Ghosts();

				// Apply profile updates for users.
				Network.Accept_Pending_Profile_Changes(); 
				
				// Any players spawned? If so start the countdown.
				if (Scene.Find_Actors(typeof(Human_Player)).Length() > 0)
				{
					change_state(State_Count_Down);
					return;
				}	
			}
			
			// Set HUD title.
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			game_hud.Set_Objective_Text("Waiting For Players" , false);			
			
			// Tick base code.
			Update_Base();
		}
	}
	
	// -------------------------------------------------------------------------------------------------
	// Countdown until we start spawning enemies.
	// -------------------------------------------------------------------------------------------------
	default state State_Count_Down
	{
		event void On_Enter()
		{		
			if (Network.Is_Server())
			{
				m_game_state = Example_Game_Mode_State.Countdown;
				m_countdown_timer = COUNTDOWN_DURATION;
			}
		}	
		event void On_Exit()
		{
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(0);		

				// Revive ghosts.
				Player_Manager.Revive_Ghosts();

				// Apply profile updates for users.
				Network.Accept_Pending_Profile_Changes(); 	
			}
			
			// Set HUD title.
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			game_hud.Set_Objective_Text("Starting in %i ..".Format(new object[] { <object><int>m_countdown_timer }), false);			
			
			// Tick base code.
			Update_Base();
		}
		event void On_Tick()
		{
			m_countdown_timer -= ;
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Keeps spawning zombies until the player dies!
	// -------------------------------------------------------------------------------------------------
	state State_Spawning
	{
		event void On_Enter()
		{ 
		} 
		event void On_Tick()
		{
		}	
	}
 
	// -------------------------------------------------------------------------------------------------
	// User has died, shows the game over screen/bonuses.
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

			Show_Game_Over();
		}
	}
}