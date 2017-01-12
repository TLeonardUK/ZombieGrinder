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
using game.weapons.enemies.weapon_bouncer_zombie;
using game.difficulty.difficulty_manager;
 
[
	Placeable(false), 
	Name("Bouncer Zombie"), 
	Description("A blob that throws himself at the player.") 
]
public class Bouncer_Zombie : Enemy
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	const float START_HEALTH			= 80.0f;
	const float HIT_OFFSET_FX_AMOUNT	= 6.0f;
	const int	SCORE_INCREASE			= 150;
	const int	HIT_SCORE_INCREASE		= 1;
	const int	XP_INCREASE				= 1000;

	const float	JUMP_HIGH_ANIM_DURATION	= 0.25f;

	const string WALK_ANIMATION			= "enemy_bouncer_zombie_walk_";

	private string m_active_walk_animation;

	const float JUMP_HEIGHT = 24.0f;
	const float JUMP_SCALE_REDUCTION = 0.2f;

	private bool bJumpingLastFrame = false;

	public serialized(1) bool bJumping = false;
	public serialized(1) bool bCharging = false;
	public serialized(1) float m_jump_time_elapsed = 0.0f;

	private float m_jump_delta = 0.0f;
	private float m_lerp_jump_delta = 0.0f;

	const float HURT_EVENT_RADIUS = 32.0f;
	
	string BLOOD_EFFECT	= "bouncer_zombie_blood_splat_trail";

	private Vec3 m_last_blood_location = new Vec3(0.0f);
	private float m_time_since_last_blood = 0.0f;

	const float BLOOD_DROP_POSITION_DELTA = 6.0f;
	const float BLOOD_DROP_TIME_DELTA = 1.0f;
    
    private bool bHasGivenAilmentScore = false;
	const int AILMENT_HIT_SCORE_INCREASE = 1000;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Bouncer_Zombie()
	{
		Bounding_Box			= Vec4(0, 0, 32, 32);
		Depth_Y_Offset			= collision.Area.Y + collision.Area.Height;
		m_active_walk_animation	= WALK_ANIMATION;

		if (Network.Is_Server())
		{
			// Setup base health.
			Setup_Health(START_HEALTH * Difficulty_Manager.Enemy_Health_Multiplier);

			// Give fodder zombie a standard weapon.
			Give_Weapon(typeof(Weapon_Bouncer_Zombie));
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
			FX.Spawn_Untracked("bouncer_zombie_die_explosion", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
		}
		else
		{
			FX.Spawn_Untracked("bouncer_zombie_die",  this.Collision_Center, 0.0f, Layer);
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

	public override float Get_Damage_Multiplier()
	{	
		return Difficulty_Manager.Difficulty_Damage_Curve;
	}
	
	public override bool Can_Target()
	{
		return true;
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

			if (bJumping && !bJumpingLastFrame)
			{				
				FX.Spawn_Untracked("bouncer_zombie_start_jump", this.Collision_Center, 0.0f, this.Layer, this);	
			}
			else if (!bJumping && bJumpingLastFrame)
			{
				FX.Spawn_Untracked("bouncer_zombie_end_jump", this.Collision_Center, 0.0f, this.Layer, this);	
			}
			bJumpingLastFrame = bJumping;

			if (!bJumping)
			{
				Drop_Blood();
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

		// Take no damage while jumping.
		if (bJumping)
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
				FX.Spawn_Untracked("bouncer_zombie_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Show HP reduction.
				FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Get_Throttled_Damage());
			}

			// Flash in damage.	
			if (m_status_ailment != StatusAilmentType.Shocked)
			{
				body_sprite.Play_Oneshot_Animation("enemy_bouncer_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
			}
	
			// Notify nearby that we have been hurt.
			Event_Hear_Sound hear_evt = new Event_Hear_Sound();
			hear_evt.Instigator = instigator;
			hear_evt.Fire_In_Radius(Center, HURT_EVENT_RADIUS);			
            	
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
				if (Health <= 0.0f)
				{
					Die(instigator, weapon_item);
				}
			}
		}

		return accepted;
	}
	
	// -------------------------------------------------------------------------
	//  Communication with controller.
	// -------------------------------------------------------------------------
	public void Begin_Charge()
	{
		bCharging = true;
	}
	public void Finish_Charge()
	{
		bCharging = false;
	}
	public void Begin_Jump()
	{
		bJumping = true;
		m_jump_time_elapsed = 0.0f;
		m_jump_delta = 0.0f;
		m_lerp_jump_delta = 0.0f;
	}
	public void Finish_Jump()
	{
		bJumping = false;
		m_jump_delta = 0.0f;
		m_lerp_jump_delta = 0.0f;
	}
	public void Update_Jump_Delta(float d)
	{
		m_jump_delta = d;
		m_lerp_jump_delta = d;
	}

    public void Reset_Jump_State()
    {
		bCharging = false;
		bJumping = false;
		m_jump_delta = 0.0f;
		m_lerp_jump_delta = 0.0f;
    }
	
	void Drop_Blood()
	{
		float pos_delta = (m_last_blood_location - this.Position).Length();
		if (pos_delta > BLOOD_DROP_POSITION_DELTA || m_time_since_last_blood > BLOOD_DROP_TIME_DELTA)
		{			
			FX.Spawn_Untracked(BLOOD_EFFECT, this.Collision_Center, 0, this.Layer);
			m_last_blood_location = this.Position;
			m_time_since_last_blood = 0.0f;
		}

		m_time_since_last_blood += Time.Get_Delta_Seconds();
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
			body_sprite.Animation = "enemy_bouncer_zombie_shocked_" + Direction_Helper.Mnemonics[Current_Direction];
			body_sprite.Animation_Speed = 1.0f;
		}		
		else if (bJumping)
		{
			if (m_jump_time_elapsed > JUMP_HIGH_ANIM_DURATION)
			{
				body_sprite.Animation = "enemy_bouncer_zombie_spin";
				body_sprite.Animation_Speed = 1.0f;
			}
			else
			{
				body_sprite.Animation = "enemy_bouncer_zombie_high_" + Direction_Helper.Mnemonics[Current_Direction];
				body_sprite.Animation_Speed = 1.0f;
			}
		}
		else if (bCharging)
		{
			body_sprite.Animation = "enemy_bouncer_zombie_low_" + Direction_Helper.Mnemonics[Current_Direction];
			body_sprite.Animation_Speed = 1.0f;
		}
		else
		{
			body_sprite.Animation = m_active_walk_animation + Direction_Helper.Mnemonics[Current_Direction];
			body_sprite.Animation_Speed = 1.0f;
		}
		
		body_sprite.Color = Math.Lerp(body_sprite.Color, Get_Ailment_Color(), 0.2f);
		
		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;

		// Bob shadow by jump delta.
		float height_delta = 0.0f;
		
		m_lerp_jump_delta = Math.Lerp(m_lerp_jump_delta, m_jump_delta, 0.2f);

		if (bJumping)
		{
			height_delta = Math.Clamp(m_lerp_jump_delta, 0.0f, 1.0f);
			if (height_delta >= 0.5f)
			{
				height_delta = 1.0f - ((m_lerp_jump_delta - 0.5f) * 2.0f);
			}
			else
			{
				height_delta = m_lerp_jump_delta * 2.0f;
			}
		}

		body_sprite.Offset = Math.SmoothStep(Vec2(0.0f, 0.0f), Vec2(0.0f, -JUMP_HEIGHT), height_delta);
		shadow_sprite.Draw_Scale = Math.SmoothStep(Vec2(1.0f, 1.0f), Vec2(1.0f - JUMP_SCALE_REDUCTION, 1.0f - JUMP_SCALE_REDUCTION), height_delta);

		m_jump_time_elapsed += Time.Get_Delta_Seconds();
	}
	
    public override string Get_Hud_Head_Animation()
    {
        return "item_hat_enemy_bouncer";
    }

	// -------------------------------------------------------------------------
	//  Replication
	// -------------------------------------------------------------------------
	replicate(ReplicationPriority.High)
	{
		bCharging
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Absolute;
		}
		bJumping
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Absolute;
		}
		m_jump_time_elapsed
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Absolute;
		}
		m_jump_delta
		{
			Owner = ReplicationOwner.ObjectOwner;
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
			Area			= Vec4(1, 21, 30, 18);
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
