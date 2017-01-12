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

public enum Dungeon_Game_State
{
	In_Progress		= 0,
	Game_Over		= 2
}

public class Dungeon_Persistent_State
{
	// Score state.
	public int	 Score;
	public float Multiplier;

	// Player states.
	public Dungeon_Persistent_Human_State[] Human_States;
}

public class Dungeon_Persistent_Human_State
{
	public int User_Net_ID;

	public float Health;
	public float Armor;
	public float Stamina;
	
	public Weapon[] Weapons;
	public int Active_Weapon_Slot;
}

[
	Placeable(true), 
	Name("Dungeon Crawler Mode"), 
	Description("Lets crawl some dungeons!") 
]
public class Dungeon_Crawler_Mode : Base_Game_Mode
{
	private Dungeon_Game_State m_game_state;
	
	private serialized(160) bool m_level_complete = false;
	private serialized(200) string m_level_complete_destination = "";
	private serialized(161) bool m_some_have_been_alive = false;

	private Dungeon_Persistent_State m_peristent_state = null;
	private Dungeon_Persistent_Human_State[] m_human_persist_states = null;

	public serialized(900) bool IsDark = true;

	public serialized(999) int Start_Score = 0;
    
    public serialized(3000) bool bLimitEveryone = false;

	Dungeon_Crawler_Mode()
	{
		Enemy_Manager.Reset();
        Player_Manager.Reset();
	}

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
		return "#scoreboard_title_find_the_exit";
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
			float mult = Scene.Get_Map_Dungeon_Level();
			Difficulty_Manager.Set_Difficulty(2.0f + (mult * 4.0f));

			Weapon_Drops				= false;
			Weapon_Drops_From_Players	= false;

			// Started in game-over?
			if (!Network.Is_Server())
			{
				if (m_game_state == Dungeon_Game_State.Game_Over)
				{
					change_state(State_Game_Over);
				}
			}
		}

		event void On_Tick()
		{	
			// Daaaaaark.
			if (IsDark)
			{
				Scene.Set_Ambient_Lighting(Vec4(0.05f, 0.05f, 0.05f, 1.0f));
				Scene.Set_Clear_Color(Vec4(0.0f, 0.0f, 0.0f, 1.0f));
			}
			else
			{
				Scene.Set_Ambient_Lighting(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
				Scene.Set_Clear_Color(Vec4(0.133f, 0.694f, 0.29f, 1.0f));
			}

			// Update HUD countdown.
			Game_HUD game_hud = <Game_HUD>Get_HUD();
			game_hud.Set_Objective_Text(Locale.Get("#dungeon_game_mode_level").Format(new object[] { <object>(Scene.Get_Map_Dungeon_Level() + 1) }), false);

			// Change state?	
			if (Network.Is_Server())
			{
				// Spawn any players that are dead.
				Player_Manager.Spawn_Humans(0, false, false, true, true);		

				// Despawn enemies that are far away from anyone.	
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
                        Game_Over(false, "");
						//change_state(State_Game_Over);
					}
				}
				else
				{
					m_some_have_been_alive = true;
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
        if (m_game_state == Dungeon_Game_State.Game_Over)
        {
            return;
        }

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
				// Store persistent states.
				Store_Human_Persist_States();

				// Kill all incapacitated players off.
				Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
				foreach (Human_Player other_player in potential_revivers)
				{
					other_player.SuccessDie();
				}

				m_game_state = Dungeon_Game_State.Game_Over;
				Log.Write("[DC] Game Over!");	
			}

			if (m_level_complete)
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_win");
			}
			else
			{
				Audio.Play2D("sfx_game_modes_capture_the_bag_capture_lose");
			}

			Reset_Game_Over();

			if (m_level_complete)
			{
				Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_base"), Score);

				if (Highest_Multiplier >= 2.0f)
				{
					Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_multiplier"), (<int>(Highest_Multiplier - 1.0f)) * Multiplier_Bonus);
				}
			    if (!Network.Is_Dedicated_Server())
			    {
					if (Network.Get_Primary_Local_User().Kills > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_kills"), Math.Min(Max_Kill_Bonus, Network.Get_Primary_Local_User().Kills * Kill_Bonus));
					}
					if (Network.Get_Primary_Local_User().Revives > 0)
					{
						Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_revives"), Math.Min(Network.Get_Primary_Local_User().Revives, Revive_Bonus_Max_Count) * Revive_Bonus);
					}
                }
				if (Scene.Get_Map_Dungeon_Level() > 0)
				{
					int Score_Addition = Math.Max(Score - Start_Score, 0);
					float Mult = Math.Clamp(1.0f + ((Scene.Get_Map_Dungeon_Level() + 1) * 0.1f), 1.0f, 5.0f);
					int Bonus = (Score_Addition * Mult) - Score_Addition;
					Log.Write("Scored Gained This Level = "+Score_Addition+", Level="+Scene.Get_Map_Dungeon_Level()+" Multiplier = "+Mult+", Bonus = "+Bonus);
					Add_Game_Over_Bonus(Locale.Get("#game_over_bonus_dungeon_level"), Bonus);
				}
			}

			Show_Game_Over(m_level_complete ? Locale.Get("#game_over_level_complete_title") : "", m_level_complete_destination);
		}
	}
	
	// -------------------------------------------------------------------------------------------------
	// Persistent state.
	// -------------------------------------------------------------------------------------------------	
	public override object Persist_State()
	{
		Dungeon_Persistent_State s = new Dungeon_Persistent_State();
		s.Score = Get_Game_Over_Score();
		s.Multiplier = Multiplier;
		s.Human_States = m_human_persist_states;
		return s;
	}

	public void Store_Human_Persist_States()
	{
		// Grab all non-incapped humans, and store their state.
		Actor[] actors = Scene.Find_Actors(typeof(Human_Player));
		Human_Player[] humans = new Human_Player[0]; 
		foreach (Actor human in actors)
		{
			Human_Player player = <Human_Player>human;
			if (!player.Is_Incapacitated())
			{
				humans.AddLast(player);
			}
		}
		
		Log.Write("Persisting the state of "+humans.Length()+" alive players.");

		// Create states for each human.
		m_human_persist_states = new Dungeon_Persistent_Human_State[humans.Length()];
		int state_index = 0;

		foreach (Human_Player player in humans)
		{
			Dungeon_Persistent_Human_State hs = new Dungeon_Persistent_Human_State();

			hs.User_Net_ID = player.Owner.Net_ID;
			hs.Health = player.Health;
			hs.Armor = player.Armor;
			hs.Stamina = player.Stamina;
			hs.Active_Weapon_Slot = player.Active_Weapon_Slot;
			hs.Weapons = player.m_weapon_slots;

			m_human_persist_states[state_index] = hs;
			state_index++;
		}
	}

	public override void Restore_State(object inState)
	{
		Dungeon_Persistent_State s = <Dungeon_Persistent_State>inState;
		if (s != null)
		{
		    Log.Write("Restoring the state of "+s.Human_States.Length()+" alive players.");

			Score = s.Score;
			Start_Score = s.Score;
			Multiplier = s.Multiplier;
			
			m_peristent_state = s;
		}
		else
		{
			m_peristent_state = null;
		}
	}
	
	public override void Setup_Player_Weapons(Pawn actor, Profile profile)
	{	
		if (m_peristent_state != null && m_peristent_state.Human_States != null)
		{
			Dungeon_Persistent_Human_State hs = null;

			foreach (Dungeon_Persistent_Human_State other in m_peristent_state.Human_States)
			{
				if (other.User_Net_ID == actor.Owner.Net_ID)
				{
					hs = other;
					break;
				}
			}

			if (hs != null)
			{
				Log.Write("Restoring persistent state for user '" + actor.Owner.Username + "'.");
				
				actor.Health = hs.Health;
				actor.Armor = hs.Armor;
				actor.Stamina = hs.Stamina;
				
				// Add weapons.
				for (int i = 0; i < hs.Weapons.Length(); i++)
				{
					if (hs.Weapons[i] != null)
					{
						actor.Give_Weapon(hs.Weapons[i].GetType(), hs.Weapons[i].Primary_Color, false, hs.Weapons[i].Upgrade_Ids, hs.Weapons[i].Upgrade_Item_Archetype);
						if (hs.Weapons[i].Ammo_Modifier != null)
						{
							actor.Give_Weapon_Modifier(hs.Weapons[i].Ammo_Modifier.GetType());
						}
						actor.Set_Weapon_Ammo_State(i, hs.Weapons[i].Clip_Ammo, hs.Weapons[i].Reserve_Ammo);
					}
				}

				// Select specific slot.
				actor.Set_Active_Weapon_Slot(hs.Active_Weapon_Slot);

				return;
			}
		}

		// Use default weapons.
		Log.Write("Using default state for user '" + actor.Owner.Username + "'.");
		base.Setup_Player_Weapons(actor, profile);
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
		Start_Score
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}