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
using game.profile.profile;
using game.weapons.pistol.weapon_pistol;
using game.effects.health_warning_post_process_fx; 
using game.difficulty.difficulty_manager;

[
	Placeable(false), 
	Name("Human Ghost"), 
	Description("Ghostly human, woooooooooo.") 
]
public class Human_Player : Pawn
{  
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	private Direction  m_old_direction 		= Direction.S;
	private bool 	   m_old_is_idle 		= false;
	private bool 	   m_is_idle 			= false;
	private Vec3	   m_old_position;
	 
	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float SPAWN_INVISIBLE_DELAY 	=  0.30; // seconds
	const float SPRITE_DEPTH_BIAS 		=  0.01;
	const float START_HEALTH			=  100.0f;
	const float HIT_OFFSET_FX_AMOUNT	=  0.0f;
	const int	DIE_SCORE_INCREASE		= -10000;
	const int	HIT_SCORE_INCREASE		= -100;

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
			Setup_Health(START_HEALTH);
		}
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
	
	// -------------------------------------------------------------------------
	// Death work.
	// -------------------------------------------------------------------------
	override void On_Death(Actor instigator)
	{
		// Spawn a death effect.
		FX.Spawn_Untracked("human_die", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);

		// Notify everyone of what happened.
		Event_Pawn_Killed evt = new Event_Pawn_Killed();
		evt.Dead			= this;
		evt.Killer			= instigator;
		evt.Score_Increase	= DIE_SCORE_INCREASE;
		evt.Fire_Global();
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
			
			// Disable particle collector if not local. (Used to pickup coins).
			if (this.Owner != null && !this.Owner.Is_Local)
			{
				particle_collector.Is_Paused = true;
			}

			// Spawn a player-spawned effect over the player.
			FX.Spawn_Untracked("human_spawn", Position, Rotation, Layer);

			// Wait a little while before updating player sprites and
			// making them visible. Makes it look like we are spawning 
			// via the particle fx.
			Time.Sleep(SPAWN_INVISIBLE_DELAY);
		
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
			Tick_Weapon();
			
			// If local user deal with collectibles.
			if (this.Owner != null && this.Owner.Is_Local)
			{
				int picked_up_coins = particle_collector.Get_Collectod(ParticleCollectionType.Coins);
				if (picked_up_coins > 0)
				{
					this.Owner.Pickup_Coins(picked_up_coins);
				}
			}

			// Activate pylons if we touch them.			
			Path_Pylon pylon = <Path_Pylon>Pathing.Get_Pylon_By_Position(this.Collision_Center);
			if (pylon != null)
			{
				pylon.Activated_By_Player = true;		
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
			CollisionDamageType type)
		{
			bool accepted = Apply_Damage(component, instigator, position, dir, damage, type);
			if (accepted == true)
			{
				// Scale damage by difficulty - TODO: Enemies should probably have damage-scales applied to weapons, not humans.
				damage *= Difficulty_Manager.Difficulty_Damage_Curve;

				// Spawn a hit effect.
				FX.Spawn_Untracked("human_hit", position.Offset(dir, HIT_OFFSET_FX_AMOUNT), dir, Layer);
			
				// Notify everyone of what happened.
				Event_Pawn_Hurt evt = new Event_Pawn_Hurt();
				evt.Hurt			= this;
				//evt.Killer		= m_death_instigator;
				evt.Score_Increase	= HIT_SCORE_INCREASE;
				evt.Fire_Global();

				// Play random grunt.
				Audio.Play3D("sfx_objects_avatars_hurt_" + Math.Rand(1, 4), Position);

				// Flash in damage.
			//	body_sprite.Play_Oneshot_Animation("enemy_fodder_zombie_hurt_" + Direction_Helper.Mnemonics[Current_Direction], 1.0f);
		
				// Show HP reduction.
				FX.Spawn_Untracked("damage_score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>damage);

				// Flash hurt effect.
				if (this.Owner != null && this.Owner.Is_Local == true)
				{
					Health_Warning_Post_Process_FX fx = <Health_Warning_Post_Process_FX>Post_Process_FX.Get_Instance(typeof(Health_Warning_Post_Process_FX));
					fx.Show_Harm_Effect(damage);
				}

				// Deeeeead.
				if (Health <= 0.0f)
				{
					Die(instigator);
				}
			}

			return accepted;
		}
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
	
	void Update_Sprites()
	{
		float anim_speed 						= m_is_idle ? 0.0 : 1.0;

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
			if (main_anim != "" || oneshot_anim != "")
			{    
				body_sprite.Animation = main_anim;
				if (oneshot_anim != "")
				{
					body_sprite.Play_Oneshot_Animation(oneshot_anim, 1.0);
				}

				// Place above/below head based on direction.
				if (weapon.Direction_Layers[Current_Direction] == true)
				{
					body_sprite.Depth_Bias = SPRITE_DEPTH_BIAS * 4;
				}
				else
				{
					body_sprite.Depth_Bias = SPRITE_DEPTH_BIAS;			
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
		if (accessory_item != null)
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
		
		// Make shadow visible.
		shadow_sprite.Frame = "actor_shape_oval_0";
		shadow_sprite.Visible = true;
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
		}
		
		SpriteComponent legs_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = 0.0;
		}
		
		SpriteComponent body_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS;
		}
		
		SpriteComponent head_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 2;
		}
		SpriteComponent head_tint_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 3;
		}
		
		SpriteComponent accessory_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 5;
		}
		SpriteComponent accessory_tint_sprite
		{
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Depth_Bias = SPRITE_DEPTH_BIAS * 6;
		}

		ParticleCollectorComponent particle_collector
		{
			Radius		= 48.0f;
			Strength	= 70.0f;
			Offset		= Vec3(32.0f, 64.0f);
			Is_Paused	= false;
		}

		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(26, 43, 10, 14);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Player;
			Collides_With	= CollisionGroup.Environment;
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
	}
}
