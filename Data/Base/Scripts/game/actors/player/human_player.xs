// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.network;
using system.components.sprite;
using system.components.collision;
using system.components.particle_collector;
using system.actors.actor;
using system.time;
using system.fx;
using system.pathing;
using system.actors.pawn;
using game.actors.markers.path_pylon;
using game.actors.interactable.interactable;
using game.profile.profile;
using game.weapons.pistol.weapon_pistol;
using game.effects.health_warning_post_process_fx; 
using game.difficulty.difficulty_manager;
using game.modes.base_game_mode;
using game.cheats.cheat_manager;

[
	Placeable(false), 
	Name("Human Player"), 
	Description("Human player character.") 
]
public class Human_Player : Pawn, Interactable
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	private Direction  m_old_direction 					= Direction.S;
	private bool 	   m_old_is_idle 					= false;
	private bool 	   m_is_idle 						= false;
	private Vec3	   m_old_position;
	private bool	   m_incapacitated					= false;
	private float	   m_ghost_bob_offset				= 0.0f; 
	private int		   m_ghost_bob_direction			= 0; 
	private float	   m_ghost_bob_time					= 0.0f;
	private float	   m_incapacitated_bleedout_time	= 0.0f;

	private float[]	   m_temporary_buffs				= new float[CollisionDamageType.COUNT];
	private float	   m_buff_ammo_accumulator			= 0.0f;
	
	private Vec4	   m_ailment_color					= new Vec4(1.0f, 1.0f, 1.0f, 1.0f);

	private float	   m_bleedout_time					= 1.0f;

	public float	   m_movement_anim_multiplier		= 1.0f;

	private bool	   m_is_success_death				= false;

    private bool       bInCutscene                      = false;
	
    // Modifiers for challenges.
	public bool Skills_Disabled = false;
    public bool Reversed_Controls = false;
    public float Speed_Multiplier = 1.0f;

    // Fear factor used to control spawning.
    public const float FEAR_PER_KILL = 3.0f;
    public const float FEAR_PER_DEATH = 50.0f;
    public const float FEAR_PER_DAMAGE = 0.6f;
    public const float FEAR_DRAIN = 3.0f;
    public const float FEAR_DRAIN_LOW_HEALTH = 2.0f;
    public const float FEAR_DRAIN_LOW_HEALTH_THRESHOLD = 0.25f;
    public const float FEAR_GAIN_BY_PROXIMITY = 0.5f;
    public const float FEAR_GAIN_BY_PROXIMITY_DISTANCE = 96.0f;
    public const float FEAR_MAX = 100.0f;          // Need to be > FEAR_TARGET to allow it to sustain for a few seconds.
    public float Fear = 0.0f;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float SPAWN_INVISIBLE_DELAY 		=  0.30; // seconds
	const float SPRITE_DEPTH_BIAS 			=  0.01;
	const float START_HEALTH				=  100.0f;
	const float HIT_OFFSET_FX_AMOUNT		=  0.0f;
	const int	DIE_SCORE_INCREASE			= -3000;
	const float HIT_SCORE_MULTIPLIER		= 0.2f;
	const float	GHOST_BOB_MAX_OFFSET		= 6.0f;
	const float	GHOST_BOB_LERP_TIME			= 6.0f;
	const float	BLEEDOUT_TIME				= 30.0f * 1000.0f;
	const float FADEOUT_TIME				= 5.0f * 1000.0f;
	const float	REVIVE_INTERACTION_DURATION	= 5.0f * 1000.0f;
	const float REVIVE_HEALTH_FACTOR		= 0.5f;
	const float TEMPORARY_BUFF_FALLOFF		= 5.0f;
	const float DEFAULT_COIN_COLLECT_RADIUS	= 48.0f;
	const float STAMINA_RECHARGE_RATE		= 15.0f;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Human_Player()
	{
		Bounding_Box    = Vec4(0, 0, 64, 64);
		Depth_Y_Offset  = collision.Area.Y + collision.Area.Height;

		if (Network.Is_Server())
		{
			// Setup base health.
			if (Owner != null)
			{
				Setup_Health(START_HEALTH * this.Owner.Stat_Multipliers[Profile_Stat.Max_Health]);
			}
			else
			{
				Setup_Health(START_HEALTH);
			}
		}

		// No clip?
		if (Cheat_Manager.bNoClip == true)
		{
			collision.Collides_With = CollisionGroup.NONE;
		}
	}
	
	// -------------------------------------------------------------------------
	//  Interaction
	// -------------------------------------------------------------------------
	public override bool Can_Interact(Pawn other)
	{
		if (other != null &&					// Is another pawn?
			other != this &&					// Not us I hope!
			other.Team_Index == Team_Index && 	// On the same team as us?
			!bInCutscene &&                     // No cutscene actors plz.
            m_incapacitated)					// And we are incapacitated?
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#ghost_revive_prompt");
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}

	public override float Get_Duration(Pawn other)
	{
		float multiplier = 1.0f;

		Human_Player other_player = <Human_Player>other;
		if (other_player != null)
		{
			multiplier = other_player.Owner.Stat_Multipliers[Profile_Stat.Ghost_Revive_Time];
		}

		return REVIVE_INTERACTION_DURATION * multiplier;
	}

	public override void Interact(Pawn other)
	{
		Revive(other);
	}

	public bool Is_Being_Revived()
	{		
		Actor actor = Scene.Find_Actor_By_Field(typeof(Player_Human_Controller), indexof(Player_Human_Controller.m_interact_other_id), this.Net_ID);		
		return actor != null && actor.Owner != null;
	}

	public NetUser Get_Reviver_User()
	{
		Actor actor = Scene.Find_Actor_By_Field(typeof(Player_Human_Controller), indexof(Player_Human_Controller.m_interact_other_id), this.Net_ID);	
		return actor.Owner;
	}

	public float Get_Revive_Delta()
	{
		Actor actor = Scene.Find_Actor_By_Field(typeof(Player_Human_Controller), indexof(Player_Human_Controller.m_interact_other_id), this.Net_ID);
		return (<Player_Human_Controller>actor).m_interact_duration_delta;
	}

	// -------------------------------------------------------------------------
	// Movement.
	// -------------------------------------------------------------------------
	override void Move_Towards(Vec3 pos)
	{
		MoveTo(pos);
	}
	
	override void Turn_Towards(Direction dir)
	{
		Current_Direction = dir;
	}

	override bool Is_Incapacitated()
	{
		return m_incapacitated;
	}

	float Get_Incapacitated_Bleedout_Factor()
	{
        if (m_bleedout_time == 0.0f)
        {
            return 0.0f;
        }
		return Math.Clamp(m_incapacitated_bleedout_time / m_bleedout_time, 0.0f, 1.0f);
	}
	
	public override bool Can_Target()
	{
        if (Cheat_Manager.Player_Untargetable)
        {
            return false;
        }

		float target_multiplier = 1.0f;
		if (this.Owner != null)
		{
			target_multiplier = this.Owner.Stat_Multipliers[Profile_Stat.Targetable];
		}

		return !m_incapacitated && (target_multiplier == 1.0f) && !bInCutscene;
	}
	
	// -------------------------------------------------------------------------
	// Skills
	// -------------------------------------------------------------------------	
	public override void On_Fire_Begin()
	{
		// Disable ammo regen.
		if (this.Owner != null)
		{
			this.Owner.Time_Since_Last_Fire = 0.0f;
		}
	}
	public override void On_Alt_Fire_Begin()
	{
		// Disable ammo regen.
		if (this.Owner != null)
		{
			this.Owner.Time_Since_Last_Fire = 0.0f;
		}
	}

	private void Tick_Skills()
	{
		float time = Time.Get_Delta_Seconds();
		
		// Disable ammo regen.
		if (this.Owner != null && (m_firing || m_alt_firing))
		{
			this.Owner.Time_Since_Last_Fire = 0.0f;
		}

		// Regenerate health.		
		float hp_regen_fraction = Max_Health * this.Owner.Stat_Multipliers[Profile_Stat.Health_Regen];
		float hp_regen_per_frame = hp_regen_fraction * time;
		Health = Math.Min(Health + hp_regen_per_frame, Max_Health);
        
        // You die now!        
		if (this.Owner != null && this.Owner.Is_Local && hp_regen_per_frame < 0.0f)
        { 
		    if (Health <= 0.0f && !Is_Incapacitated())
		    {
	    	    Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
			    if (mode.Can_Incapacitate(this))
			    {
				    Incapacitate(this);
			    }
			    else
			    {
				    Die(this, null);
			    }
		    }
        }

		// Regenerate ammo.
		//for (int i = 0; i < Weapon_Slot.COUNT; i++)
		int i = m_active_weapon_slot;
		{
			Weapon weapon = Get_Weapon_Slot(i);
			if (weapon != null)
			{
				if (weapon.Can_Buffs_Regenerate_Ammo && weapon.Is_Reloading == false && weapon.Is_Firing() == false)
				{
					float ammo_regen_fraction = weapon.Reserve_Size * this.Owner.Stat_Multipliers[Profile_Stat.Ammo_Regen];
					float ammo_regen_per_frame = ammo_regen_fraction * time;
					weapon.Ammo_Regen_Accumulator += ammo_regen_per_frame;
					
					int add_amt = <int>weapon.Ammo_Regen_Accumulator;
					if (add_amt > 0)
					{
						weapon.Reserve_Ammo = Math.Min(weapon.Reserve_Ammo + add_amt, weapon.Reserve_Size); 
						weapon.Ammo_Regen_Accumulator -= add_amt;
					}
				}
			}
		}
	}

	// -------------------------------------------------------------------------
	// Buffs
	// -------------------------------------------------------------------------
	public float Get_Temporary_Buff(CollisionDamageType type)
	{
		return m_temporary_buffs[<int>type];
	}

	private void Apply_Temporary_Buff(CollisionDamageType type, float damage)
	{
		// Constant damage is per-second, we need to convert this back to its absolute value for buffs.
		m_temporary_buffs[<int>type] = damage / Time.Get_Delta();
	}

	private void Clear_Temporary_Buffs()
	{
		m_temporary_buffs[<int>CollisionDamageType.Buff_Ammo]		= 0.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_Health]		= 0.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_Damage]		= 1.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_Price]		= 1.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_Reload]		= 1.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_ROF]		= 1.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_Speed]		= 1.0f;
		m_temporary_buffs[<int>CollisionDamageType.Buff_XP]			= 1.0f;

		m_buff_ammo_accumulator = 0.0f;
	}

	private void Tick_Temporary_Buffs()
	{
		float lerp_delta = TEMPORARY_BUFF_FALLOFF * Time.Get_Delta_Seconds();

		m_temporary_buffs[<int>CollisionDamageType.Buff_Ammo]	= Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Ammo],		0.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_Health] = Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Health],	0.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_Damage] = Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Damage],	1.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_Price]	= Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Price],		1.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_Reload] = Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Reload],	1.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_ROF]	= Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_ROF],		1.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_Speed]	= Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_Speed],		1.0f, lerp_delta);
		m_temporary_buffs[<int>CollisionDamageType.Buff_XP]		= Math.Lerp(m_temporary_buffs[<int>CollisionDamageType.Buff_XP],		1.0f, lerp_delta);
		
		// We *shouldn't* need to sync the below buffs as they should be 
		// updating at roughly the same rate on all clients. Slightly off
		// won't make much of a difference.

		// Apply healing buff.
		base.Heal(m_temporary_buffs[<int>CollisionDamageType.Buff_Health] * Time.Get_Delta_Seconds(), 0.0f);

		// Apply ammo buff.
		m_buff_ammo_accumulator += (m_temporary_buffs[<int>CollisionDamageType.Buff_Ammo] * Time.Get_Delta_Seconds());
		Weapon weapon = Get_Active_Weapon();
		if (weapon != null && weapon.Can_Buffs_Regenerate_Ammo)
		{
			int ammo_to_add = weapon.Reserve_Size * m_buff_ammo_accumulator;
			if (ammo_to_add > 0)
			{
				weapon.Reserve_Ammo = Math.Min(weapon.Reserve_Ammo + ammo_to_add, weapon.Reserve_Size); 
				m_buff_ammo_accumulator -= (<float>ammo_to_add / <float>weapon.Reserve_Size);
			}
		}

		// Recharge sprint.
		float recharge_rate = STAMINA_RECHARGE_RATE;
        if (weapon != null)
        {
            if (weapon.Upgrade_Tree_Modifiers != null && 
                weapon.Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed] != 0.0f)
            {
                float multiplier = (1.0f + weapon.Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed]);
                recharge_rate *= multiplier;
            }
        }
        if (this.Owner != null)
		{
			recharge_rate *= this.Owner.Stat_Multipliers[Profile_Stat.Melee_Stamina];
		}

		Stamina = Math.Min(Max_Stamina, Stamina + (recharge_rate * Time.Get_Delta_Seconds()));
	}

	public override float Get_XP_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_XP] * this.Owner.Stat_Multipliers[Profile_Stat.XP];
	}
	
	public override float Get_Speed_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_Speed] * Get_Ailment_Speed_Multiplier() * this.Owner.Stat_Multipliers[Profile_Stat.Speed] * Speed_Multiplier * Cheat_Manager.Speed_Multiplier;
	}
	
	public override float Get_ROF_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_ROF];
	}

	public override float Get_Reload_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_Reload] * Cheat_Manager.Reload_Multiplier;
	}

	public override float Get_Price_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_Price];
	}
	
	public override float Get_Damage_Multiplier()
	{
		return m_temporary_buffs[<int>CollisionDamageType.Buff_Damage];
	}

	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
		// Dieing in game modes with no incap need to spawn effects here.
		if (!m_incapacitated)
		{
            Add_Fear(FEAR_PER_DEATH);

			if (m_is_success_death)
			{
				FX.Spawn_Untracked("human_die_success", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
			}
			else
			{
				// Spawn a death effect.
				if (Last_Damage_Type == CollisionDamageType.Explosive)
				{
					FX.Spawn_Untracked("human_die_explosion", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
				}
				else 
				{
					FX.Spawn_Untracked("human_die", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
				}

				// Notify everyone of what happened.
				Event_Pawn_Killed evt = new Event_Pawn_Killed();
				evt.Dead			= this;
				evt.Killer			= instigator;
				evt.Score_Increase	= DIE_SCORE_INCREASE;
                evt.Weapon_Type     = weapon_type;
				evt.Fire_Global();
			}
		}

		// Set the respawn timer.
		if (this.Owner != null)
		{
			this.Owner.Last_Spawn_Time = Time.Get_Ticks();
		}
	}

	// Dumb-ass name. Basically kills the player but shows a "warp-out" effect, rather than making it look like they die.
	public void SuccessDie()
	{
		m_is_success_death = true;
		Die(null, null);
	}

	// -------------------------------------------------------------------------
	// Initial spawn.
	// -------------------------------------------------------------------------
	default state Spawn
	{
		event void On_Tick()
		{
			// Setup base pawn.
			Setup();
            
			Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;

			// Disable particle collector if not local. (Used to pickup coins).
			if (this.Owner != null && !this.Owner.Is_Local)
			{
				particle_collector.Is_Paused = true;
			}
			else
			{
				particle_collector.Radius = DEFAULT_COIN_COLLECT_RADIUS * this.Owner.Stat_Multipliers[Profile_Stat.Gold_Magnet];
			}
			
			// Turn flash light on for dungeon modes.
			Dungeon_Crawler_Mode dungeon_mode = <Dungeon_Crawler_Mode>mode;
			if (dungeon_mode != null && dungeon_mode.IsDark)
			{		
				flashlight.Paused = false;
			}
            
			// Reset buffs.
			Clear_Temporary_Buffs();

            if (!bInCutscene)
            {
			    // Spawn a player-spawned effect over the player.
			    FX.Spawn_Untracked("human_spawn", Position, Rotation, Layer);

			    // Wait a little while before updating player sprites and
			    // making them visible. Makes it look like we are spawning 
			    // via the particle fx.
			    Time.Sleep(SPAWN_INVISIBLE_DELAY);		
            }

			// Walking time.
			change_state(Walking);
		}	
	}
		 
	// -------------------------------------------------------------------------
	// Basic walking state.
	// -------------------------------------------------------------------------
	state Walking
	{
		event void On_Enter()
		{
			Store_Last_State();
			Update_Sprites();
		}		
		event void On_Tick()
		{
            Tick_Fear();
			Tick_Skills();
			Tick_Weapon();			
			Tick_Temporary_Buffs();
            Tick_Intro();
			
			// If local user deal with collectibles.
			if (this.Owner != null && this.Owner.Is_Local)
			{
				int picked_up_coins = particle_collector.Get_Collectod(ParticleCollectionType.Coins);
				if (picked_up_coins > 0)
				{
					// TODO: Kinda shitty way to do this, should actually spawn more coins.
					picked_up_coins *= this.Owner.Stat_Multipliers[Profile_Stat.Gold_Drop];

                    if (this.Owner.Is_Premium)
                    {
                        picked_up_coins *= 2.0f;
                    }

					this.Owner.Pickup_Coins(picked_up_coins);
				}
			}

			// Activate pylons if we touch them.			
			Path_Pylon pylon = <Path_Pylon>Pathing.Get_Pylon_By_Position(this.Collision_Center);
			if (pylon != null)
			{
				pylon.Activated_By_Player = true;		
			}
			
			// Updating heading.
			if (this.Owner != null)
			{
				this.Owner.Update_Heading(this);
			}

			Calculate_Sprite_State();
			Update_Sprites();
			Store_Last_State();
		}
		event bool On_Damage(
			CollisionComponent component, 
			Actor instigator, 
			Vec3 position, 
			float dir, 
			float damage, 
			CollisionDamageType type,
			int ricochet_count,
			int penetration_count,
			Weapon_SubType weapon_subtype,
            Item_Archetype weapon_type)
		{	
			// God?
			if (Cheat_Manager.bGodMode == true)
			{
				return false;
			}

			// Local client deals with damage, not server, kinda exploitable,
			// but more responsive.
			bool bApplyDamage = (this.Owner != null && this.Owner.Is_Local == true);

			// Still alive?
			if (Health <= 0.0f)
			{
				return false;
			}

			// Treat buffs differently.
			if (type == CollisionDamageType.Buff_Ammo	||
				type == CollisionDamageType.Buff_Health	||
				type == CollisionDamageType.Buff_Damage	||
				type == CollisionDamageType.Buff_Price	||
				type == CollisionDamageType.Buff_Reload ||
				type == CollisionDamageType.Buff_ROF	||
				type == CollisionDamageType.Buff_Speed	||
				type == CollisionDamageType.Buff_XP)
			{
				Apply_Temporary_Buff(type, damage);
				return true;
			}
					
			// Ignore own projectiles.
			if (instigator == this)
			{
				// If its our own explosion, we take reduced damage, not no damage.
				if (type == CollisionDamageType.Explosive)
				{
					damage *= 0.75f;
				}
				// If its fire we take reduced damage on the condition its not
				// from the flamethrower (because its too easy to ignite yourself D:). Checking for
				// grenade subtype is hacky as all hell :)
				else if (type == CollisionDamageType.Fire && weapon_subtype == Weapon_SubType.Grenade)
				{
					damage *= 0.75f;
				}
				else
				{
					return false;
				}
			}
		
			// Scale by instigators damage multiplier.
			damage = Calculate_Damage_Multiplier(instigator, damage, weapon_subtype);

			// Accept as normal damage.
			float StartHealth = Health;
			bool accepted = Apply_Damage(component, instigator, position, dir, damage, type, ricochet_count, penetration_count, bApplyDamage, weapon_type);
			if (accepted == true)
			{
				Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;

				// We do this bit of code to prevent adding impossible damage-xp in network games where health may not change immediately.
				float accepted_damage = StartHealth - Math.Clamp(StartHealth - damage, 0.0f, Max_Health);

                // Fear for spawning.
                if (accepted_damage > 0.0f)
                {
                    Add_Fear(accepted_damage * FEAR_PER_DAMAGE);
                }

				// Throttle the FX if we are taking constant damage (acid etc)
				if (Throttle_Damage_FX(accepted_damage))
				{
					// Show HP reduction.
					float dmg = Get_Throttled_Damage();
					if (dmg > 0)
					{
						// Spawn a hit effect.
						FX.Spawn_Untracked("human_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
						// Play random grunt.
						if ((this.Owner != null && this.Owner.Is_Local == true) ||
							(instigator != null && instigator.Owner != null && instigator.Owner.Is_Local == true))
						{
                            if (this.Owner != null && this.Owner.Profile.Is_Male)
                            {
							    Audio.Play3D("sfx_objects_avatars_hurt_" + Math.Rand(1, 4), Position);
                            }
                            else
                            {
							    Audio.Play3D("sfx_objects_avatars_female_hurt_" + Math.Rand(1, 4), Position);
                            }
						}

						// Spray effect.
						FX.Spawn_Untracked("damage_score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Math.Ceiling(dmg));

						if (this.Owner != null)
						{
							this.Owner.Time_Since_Last_Damage = 0.0f;
						}
					}
					else if (dmg < 0)
					{
						// Play random grunt.
						if ((this.Owner != null && this.Owner.Is_Local == true) ||
							(instigator != null && instigator.Owner != null && instigator.Owner.Is_Local == true))
						{
							Audio.Play3D("sfx_objects_avatars_heal_1", Position);
						}

						// Spray effect.
						FX.Spawn_Untracked("heal_score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Math.Ceiling(-dmg));
					}
				}

                // Player healed, remove ailments.
                if (accepted_damage < 0.0f && m_status_ailment != StatusAilmentType.None)
                {
                    Clear_Ailment();
                }

				// Notify everyone of what happened.
				Event_Pawn_Hurt evt = new Event_Pawn_Hurt();
				evt.Hurt			= this;
                evt.Harmer          = instigator;
				evt.Score_Increase	= -(accepted_damage * HIT_SCORE_MULTIPLIER);
				evt.Damage			= accepted_damage;
                evt.Damage_Type     = type;
                evt.Weapon_Sub_Type = weapon_subtype;
				evt.Fire_Global();

				// Flash hurt effect.
				if (this.Owner != null && this.Owner.Is_Local == true)
				{
					Scene.Vibrate(this.Collision_Center, 300.0f, 0.4f);

					if (accepted_damage != 0.0f)
					{
						Health_Warning_Post_Process_FX fx = <Health_Warning_Post_Process_FX>Post_Process_FX.Get_Instance(typeof(Health_Warning_Post_Process_FX), this.Owner.Local_Index);
						if (accepted_damage >= 0)
							fx.Show_Harm_Effect(accepted_damage);
						else
							fx.Show_Heal_Effect(Math.Abs(accepted_damage));
					}
				}

				// Deeeeead.
				if (Health <= 0.0f && bApplyDamage)
				{
					if (mode.Can_Incapacitate(this))
					{
						Incapacitate(instigator);
					}
					else
					{
						Die(instigator, weapon_type);
					}
				}
			}

			return accepted;
		}
	}    
	
	// -------------------------------------------------------------------------
	// Incapacitated state.
	// -------------------------------------------------------------------------
	state Ghost
	{
		event void On_Enter()
		{
			Store_Last_State();
			Update_Ghost_Sprites();
			Clear_Temporary_Buffs();
			Clear_Ailment();

			m_incapacitated = true;
			m_bleedout_time = BLEEDOUT_TIME * this.Owner.Stat_Multipliers[Profile_Stat.Ghost_Life_Time];
			m_incapacitated_bleedout_time = 0.0f;	

			// Disable all components we don't care about.
			body_sprite.Visible				= false;
			head_sprite.Visible				= false;
			head_tint_sprite.Visible		= false;
			accessory_sprite.Visible		= false;
			accessory_tint_sprite.Visible	= false;
			particle_collector.Is_Paused	= true;
			body_hitbox.Enabled				= false;
            boid.Enabled                    = false;
		}		
		event void On_Exit()
		{
			m_incapacitated = false;

			// Re-enable all components.
			body_sprite.Visible				= true;
			head_sprite.Visible				= true;
			head_tint_sprite.Visible		= true;
			accessory_sprite.Visible		= true;
			accessory_tint_sprite.Visible	= true;
			particle_collector.Is_Paused	= (this.Owner != null && !this.Owner.Is_Local);
			body_hitbox.Enabled				= true;
            boid.Enabled                    = true;
			legs_sprite.Offset				= Vec2(0.0f, 0.0f);
			legs_sprite.Color				= Vec4(255.0f, 255.0f, 255.0f, 255.0f);
			shadow_sprite.Color				= Vec4(0.0, 0.0, 0.0, 200.0);

			Update_Sprites();
		}
		event void On_Tick()
		{
			m_incapacitated_bleedout_time += Time.Get_Frame_Time();

			float alpha = 1.0f - (Math.Max(0.0f, m_incapacitated_bleedout_time - (m_bleedout_time - FADEOUT_TIME)) / FADEOUT_TIME);
            if (m_bleedout_time == 0.0f)
            {
                alpha = 1.0f;
            }

			legs_sprite.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f * alpha);
			shadow_sprite.Color = Vec4(0.0, 0.0, 0.0, 200.0 * alpha);
                  
            Tick_Fear();
	
			Calculate_Sprite_State();
			Update_Ghost_Sprites();
			Store_Last_State();

			if (Network.Is_Server())
			{
				if (Get_Incapacitated_Bleedout_Factor() >= 1.0f)
				{
					Die(null, null);
				}
			}
		}
	}   
	
	public void HealNoFX(float health, float armor)
	{
		base.Heal(health, armor);
	}

	public override void Heal(float health, float armor)
	{
		base.Heal(health, armor);
		
		// Flash heal effect.
		if (health > 0.0f && this.Owner != null && this.Owner.Is_Local == true)
		{
			Health_Warning_Post_Process_FX fx = <Health_Warning_Post_Process_FX>Post_Process_FX.Get_Instance(typeof(Health_Warning_Post_Process_FX), this.Owner.Local_Index);
			fx.Show_Heal_Effect(health);

			Audio.Play2D("sfx_objects_avatars_heal_1");

            // Player healed, remove ailments.
            if (m_status_ailment != StatusAilmentType.None)
            {
                Clear_Ailment();
            }
		}
		
        if (Health <= 0.0f)
		{
			Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
             
			if (mode.Can_Incapacitate(this))
			{
				Incapacitate(this);
			}
			else
			{
				Die(null, null);
			}
		}
	}

	private rpc void RPC_Incapacitate(int instigator_id)
	{
		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);

		Log.Write("Pawn " + this.Net_ID + " has been incapacitated.");
		
		// Spawn a death effect.
		if (Last_Damage_Type == CollisionDamageType.Explosive)
		{
			FX.Spawn_Untracked("human_die_explosion", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
		}
		else 
		{
			FX.Spawn_Untracked("human_die", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
		}

		// Notify everyone of what happened.
		Event_Pawn_Killed evt = new Event_Pawn_Killed();
		evt.Dead			= this;
		evt.Killer			= instigator;
		evt.Score_Increase	= DIE_SCORE_INCREASE;
		evt.Fire_Global();

		// Turn into ghost!
		change_state(Ghost);
	}
	
	virtual void Incapacitate(Actor instigator)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Incapacitate(instigator != null ? instigator.Net_ID : -1);
	}
	
	private rpc void RPC_Revive(int instigator_id)
	{
		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);

		Log.Write("Pawn " + this.Net_ID + " has been revived.");
		
		// Notify everyone of what happened.
		Event_Pawn_Revived evt = new Event_Pawn_Revived();
		evt.Revived			= this;
		evt.Reviver			= instigator;
		evt.Fire_Global();

		// Refill health.
		Health = Max_Health * REVIVE_HEALTH_FACTOR;

		// Respawn!
		change_state(Spawn);
	}
	
	virtual void Revive(Actor instigator)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Revive(instigator != null ? instigator.Net_ID : -1);
	}

	// -------------------------------------------------------------------------
	//  Sprite / animation updating. 
	// -------------------------------------------------------------------------
	void Store_Last_State()
	{	
		m_old_direction = Current_Direction;
		m_old_is_idle 	= m_is_idle;
		m_old_position 	= Position;
	}
	
	void Calculate_Sprite_State()
	{
		float delta = Time.Get_Delta();
		Vec3 position_delta = Position - m_old_position;

		m_is_idle = (position_delta.X == 0 && position_delta.Y == 0);
	}
	
	void Update_Ghost_Sprites()
	{
		if (m_ghost_bob_direction == 0)
		{
			m_ghost_bob_offset = Math.SmoothStep(0.0f, GHOST_BOB_MAX_OFFSET, m_ghost_bob_time / GHOST_BOB_LERP_TIME);

			m_ghost_bob_time += Time.Get_Delta();
			if (m_ghost_bob_time >= GHOST_BOB_LERP_TIME)
			{
				m_ghost_bob_direction = 1;
				m_ghost_bob_time = 0.0f;
			}
		}
		else
		{
			m_ghost_bob_offset = Math.SmoothStep(GHOST_BOB_MAX_OFFSET, 0.0f, m_ghost_bob_time / GHOST_BOB_LERP_TIME);

			m_ghost_bob_time += Time.Get_Delta();
			if (m_ghost_bob_time >= GHOST_BOB_LERP_TIME)
			{
				m_ghost_bob_direction = 0;
				m_ghost_bob_time = 0.0f;
			}
		}
		
		// Hide tint for non-tintable weapons.
		body_sprite_tint.Visible = false;
    
		if (this.Owner != null && this.Owner.Is_Premium)
        {
            legs_sprite.Color = Vec4(255.0f, 221.0f, 127.0f, 255.0f);
        }

		legs_sprite.Animation 		= "avatar_ghost_walk_" + Direction_Helper.Mnemonics[Current_Direction];
		legs_sprite.Animation_Speed = 1.0f;
		legs_sprite.Offset			= Vec2(0.0f, m_ghost_bob_offset);
	}

	void Update_Sprites()
	{
		float walking_anim_speed_scaled			= m_movement_anim_multiplier;//(((Position - m_old_position).Length() / 16.0f) * (1.0f / Time.Get_Delta_Seconds())) * 0.25f;
		float anim_speed 						= m_is_idle ? 0.0 : walking_anim_speed_scaled;

		Item head_item 							= Get_Item_Slot(Item_Slot.Head);
		Item accessory_item 					= Get_Item_Slot(Item_Slot.Accessory);
				
		// Something to walk on.
		legs_sprite.Animation 					= "avatar_legs_" + Direction_Helper.Mnemonics[Current_Direction];
		legs_sprite.Animation_Speed 			= anim_speed;
				
		// Set the body sprite based on the weapon currently in use.
		Weapon weapon = Get_Active_Weapon();
	
		if (weapon != null)
		{
			string main_anim = weapon.Get_Animation(Current_Direction);
			string oneshot_anim = weapon.Get_Oneshot_Animation(Current_Direction);
			
			string main_anim_tint = weapon.Get_Tint_Animation(Current_Direction);
			string oneshot_anim_tint = weapon.Get_Oneshot_Tint_Animation(Current_Direction);

			if (main_anim != "" || oneshot_anim != "")
			{    
				body_sprite.Animation = main_anim;
				if (oneshot_anim != "")
				{
					body_sprite.Play_Oneshot_Animation(oneshot_anim, 1.0);
				}

				if (weapon.Is_Tintable)
				{
					body_sprite_tint.Animation = main_anim_tint;
					if (oneshot_anim_tint != "")
					{
						body_sprite_tint.Play_Oneshot_Animation(oneshot_anim_tint, 1.0);
					}
				}

				// Place above/below head based on direction.
				if (weapon.Direction_Layers[Current_Direction] == true)
				{
					body_sprite.Depth_Bias = SPRITE_DEPTH_BIAS * 7;
					body_sprite_tint.Color = weapon.Primary_Color;
					body_sprite_tint.Depth_Bias = SPRITE_DEPTH_BIAS * 8;
				}
				else
				{
					body_sprite.Depth_Bias = SPRITE_DEPTH_BIAS;			
					body_sprite_tint.Color = weapon.Primary_Color;
					body_sprite_tint.Depth_Bias = SPRITE_DEPTH_BIAS * 2;			
				}
			}
			else
			{				
				body_sprite.Animation = "avatar_body_idle_" + Direction_Helper.Mnemonics[Current_Direction];
			}
		}
		else
		{
			body_sprite.Animation = "avatar_body_idle_" + Direction_Helper.Mnemonics[Current_Direction];
		}
		
		// Apply the hat slot. NEEDS MOAR HATS.
		if (head_item != null)
		{
			head_sprite.Animation 				= head_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[Current_Direction];
			head_tint_sprite.Animation 			= head_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[Current_Direction];
			head_tint_sprite.Color				= head_item.Primary_Color;
			head_sprite.Visible 				= true;
			head_tint_sprite.Visible 			= head_item.Archetype.Is_Tintable;	
		}
		else
		{
			head_sprite.Visible 				= false;
			head_tint_sprite.Visible 			= false;	
		}
		
		// Apply the accessory item slot - glasses, horns, etc.
		if (accessory_item != null && accessory_item.Archetype.Is_Icon_Overlay)
		{
			accessory_sprite.Animation 			= accessory_item.Archetype.Icon_Animation + "_" + Direction_Helper.Mnemonics[Current_Direction];
			accessory_tint_sprite.Animation 	= accessory_item.Archetype.Icon_Animation + "_tint_" + Direction_Helper.Mnemonics[Current_Direction];		
			accessory_tint_sprite.Color			= accessory_item.Primary_Color;
			accessory_sprite.Visible 			= true;
			accessory_tint_sprite.Visible 		= accessory_item.Archetype.Is_Tintable;	
		}
		else
		{
			accessory_sprite.Visible 			= false;
			accessory_tint_sprite.Visible 		= false;
		}

		// Tint by ailment.
		m_ailment_color						= Math.Lerp(m_ailment_color,  Get_Ailment_Color(), 0.2f);
		legs_sprite.Color_Scalar			= m_ailment_color;
		body_sprite.Color_Scalar			= m_ailment_color;
		head_sprite.Color_Scalar			= m_ailment_color;
		head_tint_sprite.Color_Scalar		= m_ailment_color;
		accessory_sprite.Color_Scalar		= m_ailment_color;
		accessory_tint_sprite.Color_Scalar	= m_ailment_color;

		// Hide tint for non-tintable weapons.
		body_sprite_tint.Visible = weapon != null && weapon.Is_Tintable;

		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;

		// Track player with flashlight.
		flashlight.Offset = (Vec3(this.World_Bounding_Box.Center(), 0.0f) - this.Position);
		flashlight.Angle_Offset = Vec3(0.0f, 0.0f, Math.PI + Direction_Helper.Angles[Current_Direction]);

        // Force invisibility.
        if (Cheat_Manager.Player_Invisible)
        {
		    legs_sprite.Visible		            = false;
		    body_sprite.Visible		            = false;
		    body_sprite_tint.Visible		    = false;
		    shadow_sprite.Visible		        = false;
		    head_sprite.Visible		            = false;
		    head_tint_sprite.Visible	    	= false;
			accessory_sprite.Visible 			= false;
			accessory_tint_sprite.Visible 		= false;
        }
    }

    // -------------------------------------------------------------------------
	//  Fear based spawning.
	// -------------------------------------------------------------------------
	void Tick_Fear()
    {
        if (Network.Is_Server())
        {
            float delta_t = Time.Get_Delta_Seconds();
           
            Actor[] nearby_enemies = Scene.Find_Actors_In_Radius(typeof(Enemy), FEAR_GAIN_BY_PROXIMITY_DISTANCE, this.Center);
            if (nearby_enemies.Length() > 0)
            {
                Fear = Math.Clamp(Fear + (FEAR_GAIN_BY_PROXIMITY * nearby_enemies.Length() * delta_t), 0.0f, FEAR_MAX);
            }
            else
            {
                float drain = FEAR_DRAIN;
                if (Health < Max_Health * FEAR_DRAIN_LOW_HEALTH_THRESHOLD)
                {
                    drain = FEAR_DRAIN_LOW_HEALTH;
                }
    
                Fear = Math.Clamp(Fear - (drain * delta_t), 0.0f, FEAR_MAX);
            }
        }
        Log.WriteOnScreen("HumanFear"+this.Net_ID, Vec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, this.Owner.Username + " Fear: " + Fear);
    }	

    public void Add_Fear(float fear_inc)
    {
        if (Network.Is_Server())
        {
            Fear = Math.Min(Fear + fear_inc, FEAR_MAX);
        }
    }
    
    // -------------------------------------------------------------------------
	//  Introduction logic.
	// -------------------------------------------------------------------------
    void Begin_Intro()
    {
        bInCutscene                     = true;
		particle_collector.Is_Paused	= false;
		body_hitbox.Enabled				= false;
    }

    void End_Intro()
    {
        bInCutscene                     = false;
		particle_collector.Is_Paused	= (this.Owner != null && !this.Owner.Is_Local);
		body_hitbox.Enabled				= true;
    }

    void Tick_Intro()
    {
		if (!Cheat_Manager.bNoClip)
		{
            if (bInCutscene)
            {
                collision.Group = CollisionGroup.NONE;
            }
            else
            {
                collision.Group = CollisionGroup.Player;
            }
        }
    }
	
	// -------------------------------------------------------------------------
	//  Replication.
	// -------------------------------------------------------------------------
	replicate(ReplicationPriority.High)
	{
		Position
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
		Current_Direction
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		Health
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		Armor
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		m_is_success_death
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		bInCutscene
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		Fear
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(24, 49, 15, 11);
			Scale			= true;
			Depth_Bias 		= -128.0;
			Sort_By_Transparency = true;
			Allow_Static = false;
		}

		EffectComponent flashlight
		{
			Effect_Name = "flashlight";
			Visible = true;
			Paused = true;
		}
		
		SpriteComponent legs_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = 0.0;
			Offset = Vec2(0.0f, -1.0f);
			Allow_Static = false;
		}
		
		SpriteComponent body_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS;
			Offset = Vec2(0.0f, -1.0f);
			Allow_Static = false;
		}
		SpriteComponent body_sprite_tint
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 2;
			Offset = Vec2(0.0f, -1.0f);
			Allow_Static = false;
		}
		
		SpriteComponent head_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 3;
			Allow_Static = false;
		}
		SpriteComponent head_tint_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 4;
			Allow_Static = false;
		}
		
		SpriteComponent accessory_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 5;
			Allow_Static = false;
		}
		SpriteComponent accessory_tint_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 6;
			Allow_Static = false;
		}

		ParticleCollectorComponent particle_collector
		{
			Radius		= DEFAULT_COIN_COLLECT_RADIUS;
			Strength	= 90.0f;
			Offset		= Vec3(32.0f, 64.0f);
			Is_Paused	= false;
		}

		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(26, 44, 10, 11);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Player;
			Collides_With	= CollisionGroup.Environment;
            Smoothed        = true;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(21, 29, 21, 32);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Player;
			Collides_With	= CollisionGroup.Damage;
		}

		// Boids for seperation from enemies.
		BoidComponent boid
		{
			Center				= Vec2(31, 55);				
			Neighbour_Distance	= 72.0f;
			Seperation_Distance	= 16.0f;
			Maximum_Speed		= 75.0f;
			Maximum_Force		= 75.0f;
			Cohesion_Weight		= 0.0f;
			Avoidance_Weight	= 0.0f;
			Seperation_Weight	= 75.0f;
		}
	}
}
