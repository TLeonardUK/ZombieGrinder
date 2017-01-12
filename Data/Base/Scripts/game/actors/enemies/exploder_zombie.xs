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
using game.weapons.enemies.weapon_exploder_zombie;
using game.difficulty.difficulty_manager;
 
[
	Placeable(false), 
	Name("Exploder Zombie"), 
	Description("Booooooooooomer. >_>") 
]
public class Exploder_Zombie : Enemy
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	const float START_HEALTH			= 65.0f;
	const float HIT_OFFSET_FX_AMOUNT	= 6.0f;
	const int	SCORE_INCREASE			= 100;
	const int	HIT_SCORE_INCREASE		= 1;
	const int	XP_INCREASE				= 1000;

	const string WALK_ANIMATION			= "enemy_exploder_zombie_walk_";

	const float HURT_EVENT_RADIUS		= 32.0f;

	Vec4 body_tint_base_color;

	//const float SCREENSHAKE_DURATION	= 1.5f;
	//const float SCREENSHAKE_INTENSITY	= 8.0f;

	// -------------------------------------------------------------------------
	//  Coonstructor.
	// -------------------------------------------------------------------------
	Exploder_Zombie()
	{
		Bounding_Box			= Vec4(0, 0, 48, 48);
		Depth_Y_Offset			= collision.Area.Y + collision.Area.Height;

		body_tint_base_color = Vec4(Math.Rand(0.0f, 255.0f), 
									   Math.Rand(0.0f, 255.0f), 
									   Math.Rand(0.0f, 255.0f), 
									   255.0f);

		body_tint_sprite.Color = body_tint_base_color;

		if (Network.Is_Server())
		{
			// Setup base health.
			Setup_Health(START_HEALTH * Difficulty_Manager.Enemy_Health_Multiplier);

			// Give fodder zombie a standard weapon.
			Give_Weapon(typeof(Weapon_Exploder_Zombie));
			Set_Active_Weapon_Slot(Weapon_Slot.Melee);
		}
	}

	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
		// Spawn a death effect.
		FX.Spawn_Untracked("exploder_zombie_die",  this.Collision_Center, 0.0f, Layer, this);
		
		// Shake screen.		
		//Camera.Shake_All(SCREENSHAKE_DURATION, SCREENSHAKE_INTENSITY, Position, null);
		
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
	// Movement.
	// -------------------------------------------------------------------------
	void Explosion_Warning(bool critical)
	{
		if (critical == true)
		{
			body_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
			body_tint_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction] + "_tint", 1.0f);
		}
		else
		{
			body_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_charge_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
			body_tint_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_charge_" + Direction_Helper.Mnemonics[Current_Direction] + "_tint", 1.0f);
		}
	}
	
	rpc void RPC_Explode(int instigator_id, int weapon_type_id)
	{
		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);
        Item_Archetype weapon_type = weapon_type_id == -1 ? null : Item_Archetype.Find_By_Type(Type.Find(weapon_type_id));
		
		if (instigator != null && Network.Is_Server())
		{
			// Kill dat pawn'
			Die(instigator, weapon_type);
		}
	}

	void Explode(Actor instigator)
	{
		if (Network.Is_Server())
		{
			// Kill dat pawn'
			Die(instigator, null);
		}
	}
	
	// This is a bit crude, we hook into the fire event, when it happens we blow ourselves up.
	override void On_Fire_Begin()
	{
		Explode(null);
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
			
			// TODO: Spawn from ground etc.			

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
		float anim_speed = m_is_idle ? 0.0 : 1.0;

		Weapon weapon = Get_Active_Weapon();
	
		if (weapon != null)
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
					body_tint_sprite.Play_Oneshot_Animation(oneshot_anim + "_tint", 1.0);
				}
			}
			else
			{				
				body_sprite.Animation = WALK_ANIMATION + Direction_Helper.Mnemonics[Current_Direction];
				body_sprite.Animation_Speed = anim_speed;
			}
		}
		else
		{
			body_sprite.Animation = WALK_ANIMATION + Direction_Helper.Mnemonics[Current_Direction];
			body_sprite.Animation_Speed = anim_speed;
		}
		
		// Apply tint.
		body_tint_sprite.Animation =  body_sprite.Animation + "_tint";
		body_tint_sprite.Animation_Speed = anim_speed;

		body_sprite.Color = Math.Lerp(body_sprite.Color, Get_Ailment_Color(), 0.2f);
		body_tint_sprite.Color = Math.Lerp(body_tint_base_color, Get_Ailment_Color(), 0.2f);

		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;
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
				FX.Spawn_Untracked("exploder_zombie_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Show HP reduction.	
				FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Get_Throttled_Damage());
			}
			
			// Flash in damage.
			body_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
			body_tint_sprite.Play_Oneshot_Animation("enemy_exploder_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction] + "_tint", 1.0f);
		
			// Notify nearby that we have been hurt.
			Event_Hear_Sound hear_evt = new Event_Hear_Sound();
			hear_evt.Instigator = instigator;
			hear_evt.Fire_In_Radius(Center, HURT_EVENT_RADIUS);
				
			// Notify everyone of what happened.
			Event_Pawn_Hurt evt = new Event_Pawn_Hurt();
			evt.Hurt			= this;
			evt.Harmer			= instigator;
			evt.Damage			= damage;
			evt.Score_Increase	= HIT_SCORE_INCREASE;
            evt.Damage_Type = type;
            evt.Weapon_Sub_Type = weapon_subtype;
			evt.Fire_Global();
				
			// Fire&shock insta-kills us.
			bool bResponsibleForDeath = (((instigator == null || instigator.Owner == null) && Network.Is_Server()) || 
										 (instigator != null && instigator.Owner != null && instigator.Owner.Is_Local));
			if (bResponsibleForDeath)
			{
				if (type == CollisionDamageType.Fire || type == CollisionDamageType.Shock || Health <= 0.0f)
				{
					rpc(RPCTarget.Server, RPCTarget.None) RPC_Explode(instigator == null ? -1 : instigator.Net_ID, weapon_item == null ? -1 : weapon_item.GetType().ID);
				}
			}
		}

		return accepted;
	}
    
    public override string Get_Hud_Head_Animation()
    {
        return "item_hat_enemy_exploder";
    }

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{	
		SpriteComponent shadow_sprite
		{
			Color 			= Vec4(0.0, 0.0, 0.0, 200.0);
			Area			= Vec4(11, 34, 25, 11);
			Scale			= true;
			Depth_Bias 		= -128.0;
			Sort_By_Transparency = true;
		}		
		SpriteComponent body_sprite
		{
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias	= 0.0f;
		}
		SpriteComponent body_tint_sprite
		{
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias	= 0.01f;
		}
		
		// Movement collision.
		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(18, 33, 12, 10);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Enemy;
			Collides_With	= CollisionGroup.Environment;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(11, 7, 24, 36);
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
