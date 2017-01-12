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
using game.weapons.enemies.weapon_blobby_zombie;
using game.difficulty.difficulty_manager;
 
[
	Placeable(false), 
	Name("Small Blobby Zombie"), 
	Description("Blah") 
]
public class Small_Blobby_Zombie : Enemy
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	const float START_HEALTH			= 40.0f;
	const float HIT_OFFSET_FX_AMOUNT	= 6.0f;
	const int	SCORE_INCREASE			= 20;
	const int	HIT_SCORE_INCREASE		= 1;
	const int	XP_INCREASE				= 200;

	const float HURT_EVENT_RADIUS		= 32.0f;

	const int	VARIENT_COUNT			= 3;
	
	string WALK_ANIMATION				= "enemy_small_blobby_v1_walk";
	string HURT_ANIMATION				= "enemy_small_blobby_v1_hurt";
	string SHOCK_ANIMATION				= "enemy_small_blobby_v1_shock";

	private serialized(1) int m_varient = 0;

    private bool bHasGivenAilmentScore = false;
	const int AILMENT_HIT_SCORE_INCREASE = 1000;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Small_Blobby_Zombie()
	{
		Bounding_Box			= Vec4(0, 0, 28, 34);
		Depth_Y_Offset			= collision.Area.Y + collision.Area.Height;

		if (Network.Is_Server())
		{
			// Setup base health.
			Setup_Health(START_HEALTH * Difficulty_Manager.Enemy_Health_Multiplier);

			Give_Weapon(typeof(Weapon_Blobby_Zombie));
			Set_Active_Weapon_Slot(Weapon_Slot.Melee);

			m_varient = Math.Rand(0, VARIENT_COUNT);
		}

		WALK_ANIMATION = "enemy_small_blobby_v" + m_varient + "_walk";
		HURT_ANIMATION = "enemy_small_blobby_v" + m_varient + "_hurt";
		SHOCK_ANIMATION = "enemy_small_blobby_v" + m_varient + "_shock";
	}

	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
		// Spawn a death effect.
		FX.Spawn_Untracked("small_blobby_zombie_die",  this.Collision_Center, 0.0f, Layer);
		
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
		}
	}    
	
	// -------------------------------------------------------------------------
	//  Sprite / animation updating. 
	// -------------------------------------------------------------------------
	void Update_Sprites()
	{
		if (m_status_ailment == StatusAilmentType.Shocked)
		{
			body_sprite.Animation = SHOCK_ANIMATION;
			body_sprite.Animation_Speed = 1.09f;
		}
		else
		{
			body_sprite.Animation = WALK_ANIMATION;
			body_sprite.Animation_Speed = 1.09f;
		}

		body_sprite.Color = Math.Lerp(body_sprite.Color, Get_Ailment_Color(), 0.2f);		
	
		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;
	}

    public override string Get_Hud_Head_Animation()
    {
        return "item_hat_enemy_small_blobby";
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

		// Ignore poison.
		if (type == CollisionDamageType.Poison)
		{
			return false;
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
				FX.Spawn_Untracked("blobby_zombie_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Show HP reduction.	
				FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Get_Throttled_Damage());
			}
			
			// Flash in damage.
			body_sprite.Play_Oneshot_Animation(HURT_ANIMATION, 1.0f);
		
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

		// If damage comes from a client, the client deals with the death flag, if it comes from server
		// the server deals with the death flag.
		bool bResponsibleForDeath = (((instigator == null || instigator.Owner == null) && Network.Is_Server()) || (instigator != null && instigator.Owner != null && instigator.Owner.Is_Local));
		if (bResponsibleForDeath)
		{
			if (Health <= 0.0f)
			{
				Die(instigator, weapon_item);
			}
		}

		return accepted;
	}

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(1, 21, 26, 14);
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
			Area 			= Vec4(8, 23, 12, 10);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Environment;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 28, 34);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Damage;
		}

		// Boids for seperation. Should probably use cohesion
		// and avoidance as well, but blah.
		BoidComponent boid
		{
			Center				= Vec2(23, 24);				
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
