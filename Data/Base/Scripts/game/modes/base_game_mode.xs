// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.game_mode;
using game.drops.weapon_drop_table;
using game.drops.gem_drop_table;

[
	Placeable(true), 
	Name("Base_Game_Mode"), 
	Description("Contains basic code shared between all game modes in ZG.") 
]
public class Base_Game_Mode : Game_Mode
{
	// How much score increase is converted to multiplier.
	public const float MAX_MULTIPLIER = 20.0f;
	public const float SCORE_MULTIPLIER_CONVERSION_FACTOR = 0.0006f;
	public const float MULTIPLIER_LOST_TIME = 20.0f;

	public const float MINIMUM_SCORE_SOUND_INTERVAL = 0.1f;

	public const float MULTIPLIER_BLINK_INTERVAL_MAX = 1.3f;
	public const float MULTIPLIER_BLINK_INTERVAL_MIN = 0.2f;
	public const float MULTIPLIER_BLINK_POWER = 1.0f;

	public Vec4 NORMAL_MULTIPLIER_COLOR   = new Vec4(255.0f, 255.0f, 255.0f, 255.0f);
	public Vec4 WARNING_MULTIPLIER_COLOR  = new Vec4(255.0f, 216.0f, 11.0f, 255.0f);
	public Vec4 CRITICAL_MULTIPLIER_COLOR = new Vec4(255.0f, 64.0f, 64.0f, 255.0f);
	public Vec4 FROZEN_MULTIPLIER_COLOR	  = new Vec4(11.0f, 150.0f, 255.0f, 255.0f);
	
	// Score bonuses
	const float Round_Bonus						= 10000;		// Score bonus per round.
	const float Multiplier_Bonus				= 5000;			// Bonus for highest multiplier.
	const float Kill_Bonus						= 100.0f;		// Bonus for individual number of kills.
	const float Max_Kill_Bonus					= 50000.0f;		// Maximum bonus for klls.
	const float Revive_Bonus					= 6000.0f;		// Bonus for reviving other player.
	const int   Revive_Bonus_Max_Count			= 10;

	// Current shared score.
	public int	 Score;
	
	// Current score multiplier. Reduces over time and when the players
	// cause a foul (being hurt etc).
	public float Multiplier = 1.0f;
	
	// If set this is the main game mode of the map and will be used when displaying game mode etc.
	public serialized(400) bool Is_Main = false;

	// Game time.
	public serialized(150) float Game_Time = 0.0f;

	// These display values are lerped from/to the real ones
	// to make them change a bit more naturally.
	public int	 Display_Score;
	public float Display_Multiplier = 1.0f;
	public float Highest_Multiplier = 1.0f;

    public const float HEAL_XP_MULTIPLIER = 5.0f; // Full heal of a player is worth 500 XP. So about 5 normal enemies.

	// Color to display multiplier based on how critical it is.
	public Vec4  Multiplier_Color;

	// Ignore players weapons when they spawn.
	public bool Ignore_Player_Weapons;
	
	// Disables game-over bonuses.
	public bool No_Bonuses;
	
	// Score over time lost.
	private int	  m_last_score;
	private float m_multiplier_reduction_timer;
	private float m_blink_timer = 0.0f;
	private float m_blink_interval = 0.0f;
	private float m_multiplier_blink_power = 0.0f;
	private float m_score_sound_timer = 0.0f;
	private bool  m_multiplier_frozen = false;
	private bool m_multiplier_can_be_unfrozen = false;

	private bool m_multiplier_reduce_over_time = true;

	// Object drops.
	protected Weapon_Drop_Table m_weapon_drop_table = new Weapon_Drop_Table();
	protected Gem_Drop_Table m_gem_drop_table = new Gem_Drop_Table();
    
    public bool Can_Spectators_Control_Zombies = false;

    // Boss intro.
    public bool bBossIntroActive = false;
    public Boss BossIntroActor = null;

	// -------------------------------------------------------------------------------------------------
	// Gameplay modifiers.
	// -------------------------------------------------------------------------------------------------
	public serialized(20) bool Weapon_Drops					= true;
	public serialized(20) bool Gem_Drops					= true;
	public serialized(21) bool Weapon_Drops_From_Players	= false;
	public serialized(21) bool Gem_Drops_From_Players		= false;

    public bool Allow_Manual_Weapon_Drops = true;

	// -------------------------------------------------------------------------------------------------
	// Base update.
	// -------------------------------------------------------------------------------------------------	
	Base_Game_Mode()
	{
        Difficulty_Manager.Reset();
		Skill_Effect_Manager.Reset();
        Can_Spectators_Control_Zombies = Cheat_Manager.Allow_Human_Zombie_Spawns_In_All_Modes || (!Is_PVP() && Network.Get_Local_Users().Length() <= 1); // Only allow controling of zombies if not pvp and we only have one local user (or we end up with camera issues).
	}

	public void Update_Base()
	{
		Update_Score();
		Skill_Effect_Manager.Tick();
        Difficulty_Manager.Tick();

		Game_Time += Time.Get_Delta_Seconds();

        if (!Is_PVP())
        {
            // Update HUD countdown.
            Game_HUD game_hud = < Game_HUD > Get_HUD();
            
            // Add pips for all enemies.          
            Actor[] actors = Scene.Find_Actors(typeof(Human_Player));            
            foreach (Human_Player player in actors)
            {
                if (player.Health <= 0.0f)
                {
                    Compass.Add_Pip(player.Position, "game_hud_compass_ghost", -1, Vec4(255.0f, 71.0f, 30.0f, 255.0f));
                }
                else if ((player.Health / player.Max_Health) < 0.25f)
                {
                    Compass.Add_Pip(player.Position, "game_hud_compass_health", -1, Vec4(255.0f, 122.0f, 30.0f, 255.0f));
                }
                else
                {
                    Compass.Add_Pip(player.Position, "game_hud_compass_player", -1, Vec4(188.0f, 188.0f, 188.0f, 255.0f));
                }
            }
        }
	}

    public virtual bool In_Cutscene_Mode()
    {
        Actor[] actors = Scene.Find_Actors(typeof(Player_Human_Controller));

        bool playersInIntro = false;

        foreach (Player_Human_Controller player in actors)
        {
            if (player.Is_In_Outro() && player.Owner && player.Owner.Is_Local)
            {
                playersInIntro = true;
                break;
            }   
        }             

        return bBossIntroActive || playersInIntro;
    }

	// -------------------------------------------------------------------------------------------------
	// Tinting
	// -------------------------------------------------------------------------------------------------	
	public virtual bool Should_Tint_Teams()
	{
		return false;
	}

	public virtual void Setup_Player_Weapons(Pawn actor, Profile profile)
	{		
		// Remove all weapons from actor.
		actor.Remove_All_Weapons();
	
		// Give default pistol to the actor			
		actor.Give_Weapon(typeof(Weapon_Pistol));
		
		if (!Ignore_Player_Weapons)
		{
			// Look for a weapon item for the user and given them that.
			Item weapon_item = profile.Get_Item_Slot(Item_Slot.Weapon);
			if (weapon_item != null) 
			{
				actor.Give_Weapon(weapon_item.Archetype.Weapon_Type, weapon_item.Primary_Color, false, weapon_item.Upgrade_Ids, weapon_item.Archetype);
				
				// Give weapon modifiers.
				Item[] modifiers = profile.Get_Attached_Items(weapon_item);
				foreach (Item attached in modifiers)
				{ 
					// Modifiers are always applied to equipped item, last given item is currently equipped
					// so this shouldn't cause any issues. If it does pass the slot id as well.
					actor.Give_Weapon_Modifier(attached.Archetype.Ammo_Type);			
				}
			}	

			// Give players armour if their items require it.
			if (!Is_PVP())
			{ 
				Item accessory_item = profile.Get_Item_Slot(Item_Slot.Accessory);
				if (accessory_item && accessory_item.Archetype.Initial_Armour_Amount > 0)
				{
					actor.Heal(0, accessory_item.Archetype.Initial_Armour_Amount);
				}
			}
		}

        if (Cheat_Manager.Initial_Weapon != null && actor.Owner.Local_Index == 0)
        {
            actor.Give_Weapon(Cheat_Manager.Initial_Weapon, Vec4(1.0f, 1.0f, 1.0f, 1.0f), false, new int[0], null);
        }
    }
	
	public virtual void Setup_Player_Items(Pawn actor, Profile profile)
	{
		// Remove all items from the actor.		
		actor.Remove_All_Items();
	
		// Give cosmetic items.		
		Item head_item 		= profile.Get_Item_Slot(Item_Slot.Head);
		Item accessory_item = profile.Get_Item_Slot(Item_Slot.Accessory);
		
		if (head_item != null)
		{
			if (Should_Tint_Teams())
			{
				head_item.Primary_Color = Get_Teams()[actor.Owner.Team_Index].Primary_Color * 255.0f;
			}
			actor.Give_Item(head_item);
		}
		if (accessory_item != null)
		{
			if (Should_Tint_Teams())
			{
				accessory_item.Primary_Color = Get_Teams()[actor.Owner.Team_Index].Primary_Color * 255.0f;
			}
			actor.Give_Item(accessory_item);
		}
	}

	// -------------------------------------------------------------------------------------------------
	// Score state.
	// -------------------------------------------------------------------------------------------------	
	public void Add_Score(int val)
	{
		if (val != 0)
		{
			if (val < 0)
			{
				Lose_Multiplier(false);

                if (Network.Is_Server())
		        {
				    Score = Math.Max(0, Score + val);
                }
			}
			else
			{                
		        if (Network.Is_Server())
		        {
                    Score = Math.Min(<int>(Score + (val * Multiplier)), 999999999);
				    Multiplier = Math.Clamp(Multiplier + (val * SCORE_MULTIPLIER_CONVERSION_FACTOR), 0.0f, MAX_MULTIPLIER);
				    if (Multiplier > Highest_Multiplier)
				    {
					    Highest_Multiplier = Multiplier;
				    }
                }
			}
            
		    if (Network.Is_Server())
		    {
			    if (m_multiplier_frozen && m_multiplier_can_be_unfrozen)
			    {
				    m_multiplier_frozen = false;
			    }
            }
		}
	}
	
	public override void On_Trigger()
	{
		if (Network.Is_Server())
		{
			Enabled = !Enabled;
			Log.Write("Enabled state of game mode " + this.Net_ID + " ("+this.GetType().Name+") set to " + Enabled);

			if (Enabled == true)
			{
				Scene.Active_Game_Mode = this;
			}
		}
	}

    public override int Get_Scoreboard_Is_Dead(NetUser user)
    {
		Controller controller = user.Controller;

        if (controller != null)
        {
			Pawn pawn = controller.Possessed;
		    Human_Player human = <Human_Player>pawn;
            if (human != null)
            {
                return (pawn.Health <= 0.0f);
            }
        }

        return true;
    }

	public void Lose_Multiplier(bool bPlaySound)
	{
		if (Multiplier > 1.0f)
		{
		    if (Network.Is_Server())
	    	{
    			Multiplier = 1.0f;
                Display_Multiplier = 1.0f;
            }

			if (bPlaySound)
			{
				Audio.Play2D("sfx_hud_multiplier_decrease");
			}
		}
	}

	private void Reduce_Multiplier_Over_Time()
	{
		if (m_multiplier_reduce_over_time)
		{
			if (Multiplier < 1.1f || Score > m_last_score)
			{
				m_multiplier_reduction_timer = MULTIPLIER_LOST_TIME;
			}
			else
			{
				m_multiplier_reduction_timer = Math.Max(0.0f, m_multiplier_reduction_timer - Time.Get_Delta());

				if (m_multiplier_reduction_timer <= 0.0f)
				{
					Lose_Multiplier(true);
				}
			}
		
			m_last_score = Score;
		}
	}

	protected void Freeze_Multiplier()
	{
		m_multiplier_frozen = true;
	}

	protected void Unfreeze_Multiplier()
	{
		m_multiplier_can_be_unfrozen = true;
	}

	private void Update_Score()
	{
		if (m_multiplier_frozen == false)
		{
			Reduce_Multiplier_Over_Time();
		}
		Update_Multiplier_Color();

		m_score_sound_timer -= Time.Get_Delta();

		int original_display_score = Display_Score;

		Display_Score	   = Math.Lerp(Display_Score, Score, 0.1f);
		Display_Multiplier = Math.Lerp(Display_Multiplier, Multiplier, 0.1f);

       // Log.Write("Score="+Score+"/"+Display_Score+" Multi="+Display_Multiplier+"/"+Multiplier);

		if (Display_Score == original_display_score)
		{
			Display_Score = Score;
		}
	}

	private void Update_Multiplier_Color()
	{
		if (m_multiplier_frozen == true)
		{
			Multiplier_Color = FROZEN_MULTIPLIER_COLOR;
		}
		else
		{
			// Lerp multipler from white->yellow->red
			// Blink multiplier when critical.
			if (m_multiplier_reduce_over_time)
			{
				float state_interval	 = MULTIPLIER_LOST_TIME / 20.0f;
				float critical_threshold = state_interval * 9;
				float warning_threshold  = state_interval * 12;

				m_multiplier_blink_power = Math.Lerp(m_multiplier_blink_power, 0.0f, 4.0f * Time.Get_Delta_Seconds());

				// Critical
				if (m_multiplier_reduction_timer <= critical_threshold)
				{
					float delta = m_multiplier_reduction_timer / critical_threshold;
					Multiplier_Color = Math.Lerp(CRITICAL_MULTIPLIER_COLOR, WARNING_MULTIPLIER_COLOR, delta);
					
					m_blink_timer -= Time.Get_Delta();
					if (m_blink_timer <= 0.0f)
					{
						float interval = MULTIPLIER_BLINK_INTERVAL_MIN + ((MULTIPLIER_BLINK_INTERVAL_MAX - MULTIPLIER_BLINK_INTERVAL_MIN) * delta);
						m_blink_timer = interval;
						m_blink_interval = interval;
					}

					float blink_delta = (m_blink_timer / m_blink_interval);
					Multiplier_Color = Math.Lerp(Multiplier_Color, Vec4(0.0f, 0.0f, 0.0f, 0.0f), blink_delta);

					/*
					float delta = m_multiplier_reduction_timer / critical_threshold;
					Multiplier_Color = Math.Lerp(CRITICAL_MULTIPLIER_COLOR, WARNING_MULTIPLIER_COLOR, delta) * (1.0f + m_multiplier_blink_power);
			
					*/
				}
				// Warning
				else if (m_multiplier_reduction_timer <= warning_threshold)
				{
					float delta = (m_multiplier_reduction_timer - critical_threshold) / (warning_threshold - critical_threshold);
					Multiplier_Color = Math.Lerp(WARNING_MULTIPLIER_COLOR, NORMAL_MULTIPLIER_COLOR, delta);
				}
				// Normal
				else
				{
					Multiplier_Color = NORMAL_MULTIPLIER_COLOR;
				}
			}
			else
			{
				Multiplier_Color = NORMAL_MULTIPLIER_COLOR;
			}
		}
	}

	// -------------------------------------------------------------------------------------------------
	//	Damage handling.
	// -------------------------------------------------------------------------------------------------
	virtual event void On_Pawn_Killed(Event_Pawn_Killed evt)
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

		// Add score to global.
		Add_Score(evt.Score_Increase);

        // Add fear.
        Human_Player player = <Human_Player>killer_actor;
        if (player)
        {
            player.Add_Fear(player.FEAR_PER_KILL);
        }

		// Give rewards to local players.
		if (killer_user != null && 
			killer_user.Is_Local == true)
		{    
            Network.Reset_Local_Idle_Timer();

            // Recharge skills
            killer_user.Local_Profile.Increment_Skill_Energies(Skill_Energy_Type.Enemy_Kills, 1.0f);

			if (evt.Weapon_Type)
			{
				killer_user.Local_Profile.Increment_Item_Stat(evt.Weapon_Type, Profile_Item_Stat_Type.Enemies_Killed, 1);					
			}

			if (evt.Coin_Reward != "")
			{
				FX.Spawn_Untracked(evt.Coin_Reward, Vec3(evt.Dead.World_Bounding_Box.Center(), 0.0f), 0.0f, evt.Dead.Layer);

				Event_Item_Dropped drop_evt = new Event_Item_Dropped();
				drop_evt.IsCoin = true;
				drop_evt.Fire_Global();
			}
			
			if (evt.XP_Reward != 0 && !Engine.In_Restricted_Mode())
			{
				// Multiply XP by users buff.
				Pawn pawn = <Pawn>killer_actor;
				float xp_multiplier = 1.0f;
				if (pawn != null)
				{
					xp_multiplier = pawn.Get_XP_Multiplier();
				}
                if (killer_user.Is_Premium)
                {
                    xp_multiplier *= 2.0f;
                }

				// Level up the user.
				bool leveled_up = killer_user.Local_Profile.Give_XP(evt.XP_Reward * xp_multiplier);
				if (leveled_up == true)
				{
					if (killer_actor != null)
					{
                        rpc(RPCTarget.All, RPCTarget.None) RPC_Show_Level_Up(killer_actor.Net_ID);
					}
				}
			}

			// Spawn some sparkly gems!
			if (Gem_Drops == true)
			{
				if ((<Enemy>evt.Dead) != null || Gem_Drops_From_Players == true)
				{
                    float rate_multiplier = 1.0f;
                    if (killer_user.Is_Premium)
                    {
                        rate_multiplier = 2.0f;
                    }

					if (m_gem_drop_table.Should_Drop(rate_multiplier))
					{
                        if (!Engine.In_Restricted_Mode())
                        {
						    Type type = m_gem_drop_table.Get_Drop();

						    Actor drop = Scene.Spawn(type, null);
						    drop.Layer = evt.Dead.Layer;
						    drop.Position = Vec3(evt.Dead.World_Bounding_Box.Center(), 0.0f) - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f);

                            Pickup pickup = <Pickup>drop;
                            if (pickup)
                            {
                                pickup.Lifetime = 120.0f;
                            }

						    Event_Item_Dropped drop_evt = new Event_Item_Dropped();
						    drop_evt.IsGem = true;
						    drop_evt.Fire_Global();
                        }
					}
				}
			}
		}

		// Spawn random weapon drops.
		if (Weapon_Drops == true && Network.Is_Server())
		{
			if ((<Enemy>evt.Dead) != null || Weapon_Drops_From_Players == true)
			{
				if (m_weapon_drop_table.Should_Drop())
				{
					Type type = m_weapon_drop_table.Get_Drop();

					Actor drop = Scene.Spawn(type, null);
					drop.Layer = evt.Dead.Layer;
					drop.Position = Vec3(evt.Dead.World_Bounding_Box.Center(), 0.0f) - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f);

                    Pickup pickup = <Pickup>drop;
                    if (pickup)
                    {
                        pickup.Lifetime = 120.0f;
                    }

                }
			}
		}

		// Boost death counter of users.
		if (evt.Dead.Owner != null && killer_actor != null) // killer=null=killed by server
		{
			evt.Dead.Owner.Deaths++;
		}

		// Boost kill counter of users if not suicide.
		if (killer_user != null && (evt.Dead.Owner == null || killer_user != evt.Dead.Owner))
		{
			killer_user.Kills++;
		}

		// Add kill feed.
		if (Network.Is_Server())
		{
			if ((<Human_Player>evt.Dead) != null && killer_actor != null)
			{
				Add_Kill_Feed(<Human_Player>evt.Dead, evt.Killer, killer_actor);
			}
		}
	}

    public rpc void RPC_Show_Level_Up(int net_id)
    {
        Actor act = Scene.Get_Net_ID_Actor(net_id);
        if (act != null)
        {
		    Effect fx = FX.Spawn("level_up", Vec3(act.World_Bounding_Box.Center(), 0.0f), 0.0f, act.Layer + 1, act);
		    fx.DisposeOnFinish = true;
		    fx.Attach_To(act);
        }
    }

	public void Add_Kill_Feed(Human_Player dead, Actor killer_imposter, Actor killer)
	{
		string dead_name = dead.Owner.Display_Username;
		string killer_name = (killer.Owner != null ? killer.Owner.Display_Username : "");

/*		if (killer_imposter != null && killer_imposter != killer)
		{
			Network.Send_Chat(ChatType.Server, Locale.Get("#chat_message_player_killed_imposter").Format(new object[] { dead_name, killer_name }));
		}
		else
		{*/
			if (killer_name == "")
			{
				Network.Send_Chat(ChatType.Server, Locale.Get("#chat_message_player_killed_enemy").Format(new object[] { dead_name }));
			}
			else
			{
				Network.Send_Chat(ChatType.Server, Locale.Get("#chat_message_player_killed").Format(new object[] { dead_name, killer_name }));
			}
			
//		}
	}

	event void On_Pawn_Hurt(Event_Pawn_Hurt evt)
	{
		// Deal with imposing-actors (turrets etc).
		Actor   killer_actor = evt.Harmer;
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
		
        if (killer_user != null && 
			killer_user.Is_Local == true)
        {
            Network.Reset_Local_Idle_Timer();
        }

        // If healing, give XP to instigator.
        if (evt.Damage < 0.0f)
        {
		    if (!Engine.In_Restricted_Mode())
		    {
			    // Multiply XP by users buff.
			    Pawn pawn = <Pawn>killer_actor;
			    float xp_multiplier = 1.0f;
			    if (pawn != null)
			    {
				    xp_multiplier = pawn.Get_XP_Multiplier();
			    }
                
			    // Level up the user.
			    if (killer_user.Is_Local)
                {
                    bool leveled_up = killer_user.Local_Profile.Give_XP((-evt.Damage) * HEAL_XP_MULTIPLIER * xp_multiplier);
			        if (leveled_up == true)
			        {
				        if (killer_actor != null)
				        {
                            rpc(RPCTarget.All, RPCTarget.None) RPC_Show_Level_Up(killer_actor.Net_ID);
				        }
			        }
                }
		    }
        }

        // Add score to global.
        Add_Score(evt.Score_Increase);
	}

	event void On_Pawn_Revived(Event_Pawn_Revived evt)
	{
		// Boost kill counter of users.
		if (evt.Reviver != null && evt.Reviver.Owner != null)
		{
			evt.Reviver.Owner.Revives++;
		}
	}
	
	public virtual bool Can_Incapacitate(Pawn other)
	{
		Human_Player player = <Human_Player>(other);
		if (player == null)
		{
			return false;
		}
		else
		{
			// If there are other alive to revive them.
			Actor[] potential_revivers = Scene.Find_Actors(typeof(Human_Player));
			foreach (Human_Player other_player in potential_revivers)
			{
				if (other_player != player && !other_player.Is_Incapacitated())
				{
					return true;
				}
			}
		}
		return false;
	}

	// -------------------------------------------------------------------------------------------------
	// Request zombie control for player.
	// -------------------------------------------------------------------------------------------------	
    public void Request_Zombie_Control(NetUser user)
    {
        rpc(RPCTarget.Server, RPCTarget.None) RPC_Request_Zombie_Control(user.Net_ID);
    }

    private rpc void RPC_Request_Zombie_Control(int net_id)
    {
        NetUser user = Network.Get_User(net_id);
        if (user)
        {
            Log.Write("User "+net_id+" is requesting zombie control while spectating.");
            user.Waiting_For_Zombie_Control = true;
        }
    }
    
	// -------------------------------------------------------------------------------------------------
	// Boss introduction logic.
	// -------------------------------------------------------------------------------------------------
    public void Begin_Boss_Intro(Boss boss)
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Begin_Boss_Intro(boss.Net_ID);
    }
    private rpc void RPC_Begin_Boss_Intro(int net_id)
    {
        Boss boss = <Boss>Scene.Get_Net_ID_Actor(net_id);
        if (boss)
        {
            bBossIntroActive = true;
            BossIntroActor = boss;
        }
    }

    public void End_Boss_Intro()
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_End_Boss_Intro();
    }
    private rpc void RPC_End_Boss_Intro()
    {
        bBossIntroActive = false;
    }

	// -------------------------------------------------------------------------------------------------
	// Replicated fields.
	// -------------------------------------------------------------------------------------------------	
	replicate(ReplicationPriority.Normal)
	{		
		Score
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		Multiplier
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		m_multiplier_frozen
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		Enabled
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}
}