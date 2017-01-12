// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.enemies.enemy;
using system.network;
using system.components.sprite;
using system.components.collision;
using system.components.boid;
using system.actors.actor;
using system.time;
using system.fx;
using system.actors.pawn;
using system.events;
using game.profile.profile;
using game.weapons.enemies.weapon_chaser_zombie;
using game.difficulty.difficulty_manager;
 
[
	Placeable(false), 
	Name("Chaser Zombie"), 
	Description("Green colored zombie that constantly chase the player.") 
]
public class Chaser_Zombie : Enemy
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	const float START_HEALTH			= 35.0f;
	const float HIT_OFFSET_FX_AMOUNT	= 6.0f;
	const float DECAPITATION_CHANCE		= 0.02f;
	const int	SCORE_INCREASE			= 50;
	const int	HIT_SCORE_INCREASE		= 1;
	const int	XP_INCREASE				= 500;

	const string WALK_ANIMATION			= "enemy_chaser_zombie_walk_";
	const string DECAP_WALK_ANIMATION	= "enemy_chaser_zombie_walk_nohead_";

	private string m_active_walk_animation;

	const float HURT_EVENT_RADIUS = 32.0f;

	const float MAX_DECAP_TIME = 5.0f;
	private float m_decap_timer = 0.0f;

	public serialized(1) bool Is_Decapitated = false;
	public Actor m_decapitator = null;
	
	public serialized(1) bool Has_Crawled_From_Earth = false;
    
    private bool bHasGivenAilmentScore = false;
	const int AILMENT_HIT_SCORE_INCREASE = 1000;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Chaser_Zombie()
	{
		Bounding_Box			= Vec4(0, 0, 32, 32);
		Depth_Y_Offset			= collision.Area.Y + collision.Area.Height;
		m_active_walk_animation	= WALK_ANIMATION;

		if (Network.Is_Server())
		{
			// Setup base health.
			Setup_Health(START_HEALTH * Difficulty_Manager.Enemy_Health_Multiplier);

			// Give fodder zombie a standard weapon.
			Give_Weapon(typeof(Weapon_Chaser_Zombie));
			Set_Active_Weapon_Slot(Weapon_Slot.Melee);
		}
	}

	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
		// Spawn a death effect.
		if (Last_Damage_Type == CollisionDamageType.Explosive)
		{
			FX.Spawn_Untracked("chaser_zombie_die_explosion", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
		}
		else if (Is_Decapitated)
		{
			FX.Spawn_Untracked("chaser_zombie_die_decapitated", this.Collision_Center, 0.0f, Layer);
		}
		else
		{
			FX.Spawn_Untracked("chaser_zombie_die",  this.Collision_Center, 0.0f, Layer);
		}
		
		// Notify everyone of what happened.
		Event_Pawn_Killed evt	= new Event_Pawn_Killed();
		evt.Dead				= this;
		evt.Killer				= instigator;
		evt.Score_Increase		= SCORE_INCREASE;
		evt.XP_Reward			= XP_INCREASE;
		evt.Coin_Reward			= "coin_splash_small";
        evt.Weapon_Type         = weapon_type;
		evt.Fire_Global();
	}
	
	void On_Decapitate(Actor instigator)
	{
		Log.Write("Pawn " + this.Net_ID + " has been decapitated.");
		m_decapitator = instigator;
		Is_Decapitated = true;
		change_state(Decapitated);
	}

	private rpc void RPC_Decapitate(int instigator_id)
	{
		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);
		On_Decapitate(instigator);
	}
	
	virtual void Decapitate(Actor instigator)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Decapitate(instigator != null ? instigator.Net_ID : -1);
	}
	
	public override float Get_Damage_Multiplier()
	{	
		return Difficulty_Manager.Difficulty_Damage_Curve;
	}
	
	public override bool Can_Target()
	{
		return !Is_Decapitated;
	}

	// -------------------------------------------------------------------------
	// Initial spawn.
	// -------------------------------------------------------------------------
	default state Spawn
	{
		event void On_Tick()
		{
			// Setup pawn.
			Setup();

			// Synced to a decap?
			if (Is_Decapitated)
			{
				On_Decapitate(null);
			}

			// Wait until crawled form the earth.
			if (Has_Crawled_From_Earth == false)
			{
				if (Spawned_By_Magic == false)
				{
					// Play crawl from earth anim.				
					body_sprite.Play_Oneshot_Animation("enemy_chaser_zombie_rise_from_ground_start", 1.0f);
				
					// Wait till we are done.			
					while (!body_sprite.Is_Oneshot_Animation_Finished)
					{
						Update_Sprites();
						shadow_sprite.Visible = false;

						Time.Sleep(0.0001f);
					}

					// Spawn "ground-closing-up" fx.
					FX.Spawn_Untracked("ground_close_up", this.Center, 0, this.Layer);
				}

				Has_Crawled_From_Earth = true;
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
			Base_Update();
			Update_Sprites();
		}		
		event void On_Tick()
		{
			Tick_Weapon();

			Base_Update();
			Update_Sprites();

			// Safety check for decapitation.
			if (Is_Decapitated)
			{
				change_state(Decapitated);
			}
		}
	}    
	
	// -------------------------------------------------------------------------
	// Decapitated state - run around in pain!
	// -------------------------------------------------------------------------
	state Decapitated
	{
		event void On_Enter()
		{
			m_active_walk_animation = DECAP_WALK_ANIMATION;
			decap_effect.Paused = false;
			decap_effect.Restart();
			
			Base_Update();
		}		
		event void On_Tick()
		{
			Tick_Weapon();
			
			Base_Update();
			Update_Sprites();

			// Decap animation run out?
			if (Network.Is_Server())
			{
				// HACK: This decap timer is to fix a bug where the decap effect finishes but never reports its
				// finished, this needs fixing properly. But I've been unable to capture the bug.
				m_decap_timer += Time.Get_Delta_Seconds();

				if (decap_effect.Is_Finished || m_decap_timer >= MAX_DECAP_TIME) 
				{
					Health = 0.0f;
					Die(m_decapitator, null);
				}
			}
		}
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
        Item_Archetype weapon_item)
	{
		// Still alive?
		if (Is_Dead)
		{
			return false;
		}

		// Ignore own projectiels.
		if (instigator == this)
		{
			return false;
		}

		// Ignore buffs.
		if (type == CollisionDamageType.Buff_Ammo	||
			type == CollisionDamageType.Buff_Health	||
			type == CollisionDamageType.Buff_Damage	||
			type == CollisionDamageType.Buff_Price	||
			type == CollisionDamageType.Buff_Reload ||
			type == CollisionDamageType.Buff_ROF	||
			type == CollisionDamageType.Buff_Speed	||
			type == CollisionDamageType.Buff_XP)
		{
			return true;
		}

		// Ignore healing based damage.
		if (damage < 0)
		{
			return false;
		}
			
		// Scale by instigators damage multiplier.
		damage = Calculate_Damage_Multiplier(instigator, damage, weapon_subtype);

		bool accepted = Apply_Damage(component, instigator, position, dir, damage, type, ricochet_count, penetration_count, true, weapon_item);
		if (accepted == true)
		{
			// Throttle the FX if we are taking constant damage (acid etc)
			if (Throttle_Damage_FX(damage))
			{
				// Spawn a hit effect.
				FX.Spawn_Untracked("chaser_zombie_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Show HP reduction.
				FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Get_Throttled_Damage());
			}

			if (!Is_Decapitated)
			{
				// Flash in damage.	
				if (m_status_ailment != StatusAilmentType.Shocked)
				{
					body_sprite.Play_Oneshot_Animation("enemy_chaser_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
				}
			}
					
			// Check for decap?
			bool decapitated = false;

			if (!Is_Decapitated)
			{
				if (type == CollisionDamageType.Projectile)
				{
					if (component == head_hitbox)
					{
						float prob = Math.Rand(0.0f, 1.0f);
						if (prob <= DECAPITATION_CHANCE)
						{
							Event_Pawn_Decapitated e = new Event_Pawn_Decapitated();
							e.Victim		= this;
							e.Decapitator	= instigator;
							e.Fire_Global();

							Decapitate(instigator);
							decapitated = true;
						}
					}
				}

				// Notify nearby that we have been hurt.
				Event_Hear_Sound hear_evt = new Event_Hear_Sound();
				hear_evt.Instigator = instigator;
				hear_evt.Fire_In_Radius(Center, HURT_EVENT_RADIUS);
            }

            // Notify everyone of what happened.
            // Note: Shock/Freeze based weapons do not give score increases on hit, otherwise they can just spam it and the enemy will not be able to respond :(
            bool bIsAilmentDamage = (type == CollisionDamageType.Ice || type == CollisionDamageType.Shock);
            if (!bIsAilmentDamage || !bHasGivenAilmentScore)
            {
                Event_Pawn_Hurt evt = new Event_Pawn_Hurt();
                evt.Hurt = this;
                evt.Harmer = instigator;
                evt.Damage = damage;
                evt.Score_Increase = bIsAilmentDamage ? AILMENT_HIT_SCORE_INCREASE : HIT_SCORE_INCREASE;
                evt.Damage_Type = type;
                evt.Weapon_Sub_Type = weapon_subtype;
                evt.Fire_Global();

                if (bIsAilmentDamage)
                { 
                    bIsAilmentDamage = true;
                    bHasGivenAilmentScore = true;
                }
            }
        	
			// Dead?			
			bool bResponsibleForDeath = (((instigator == null || instigator.Owner == null) && Network.Is_Server()) || (instigator != null && instigator.Owner != null && instigator.Owner.Is_Local));
			if (bResponsibleForDeath)
			{
				if (decapitated == false)
				{
					if (Health <= 0.0f)
					{
						Die(instigator, weapon_item);
					}
				}

				// If decapped and we get shocked, kill as we don't have a nohead shock anim ffffff.
				else if (m_status_ailment == StatusAilmentType.Shocked)
				{
					Die(instigator, weapon_item);
				}
			}
		}

		return accepted;
	}

	// -------------------------------------------------------------------------
	//  Sprite / animation updating. 
	// -------------------------------------------------------------------------
	void Update_Sprites()
	{
		float anim_speed = m_is_idle ? 0.0 : 1.0;
		bool ignore_oneshot = false;

		Weapon weapon = Get_Active_Weapon();

		// Use shocked sprite if neccessary.
		string walk_anim = m_active_walk_animation;
		if (m_status_ailment == StatusAilmentType.Shocked)
		{
			walk_anim = "enemy_chaser_zombie_shocked_";
			anim_speed = 1.0f;
			ignore_oneshot = true;
		}

		if (weapon != null && ignore_oneshot == false)
		{
			string main_anim = weapon.Get_Animation(Current_Direction);
			string oneshot_anim = weapon.Get_Oneshot_Animation(Current_Direction);

			if (main_anim != "" || oneshot_anim != "")
			{    				
				body_sprite.Animation_Speed = anim_speed;
				body_sprite.Animation = main_anim;

				if (oneshot_anim != "")
				{
					body_sprite.Play_Oneshot_Animation(oneshot_anim, 1.0);
				}
			}
			else
			{				
				body_sprite.Animation = walk_anim + Direction_Helper.Mnemonics[Current_Direction];
				body_sprite.Animation_Speed = anim_speed;
			}
		}
		else
		{
			body_sprite.Animation = walk_anim + Direction_Helper.Mnemonics[Current_Direction];
			body_sprite.Animation_Speed = anim_speed;
		}
		
		body_sprite.Color = Math.Lerp(body_sprite.Color, Get_Ailment_Color(), 0.2f);
		
		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;
	}
    
    public override string Get_Hud_Head_Animation()
    {
        return "item_hat_enemy_chaser";
    }

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(7, 25, 18, 10);
			Scale			= true;
			Depth_Bias 		= -128.0;
			Sort_By_Transparency = true;
		}		
		SpriteComponent body_sprite
		{
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias	= 0.0f;
		}
		
		// Movement collision.
		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(10, 24, 12, 8);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Environment;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(6, 14, 19, 18);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Damage;
		}
		CollisionComponent head_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(6, 1, 19, 13);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Damage;
		}
		
		EffectComponent decap_effect
		{
			Effect_Name = "chaser_zombie_decap_spray";
			Visible		= true;
			Paused		= true;
			One_Shot	= true;
			Offset		= Vec3(16, 15, 0);
		}

		// Boids for seperation. Should probably use cohesion
		// and avoidance as well, but blah.
		BoidComponent boid
		{
			Center				= Vec2(16, 28);				
			Neighbour_Distance	= 72.0f;
			Seperation_Distance	= 20.0f;
			Maximum_Speed		= 200.0f;
			Maximum_Force		= 200.0f;
			Cohesion_Weight		= 0.0f;
			Avoidance_Weight	= 0.0f;
			Seperation_Weight	= 200.0f;
		}
	}
}
