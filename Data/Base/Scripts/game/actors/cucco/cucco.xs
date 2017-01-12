// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

// TODO: Wander around
// TODO: Respond to damage

[
	Placeable(true), 
	Name("Cucco"), 
	Description("Cluck Cluck") 
]
public class Cucco : Actor, Interactable
{
	[ 
		EditType("Animation") 
	]
	public serialized(1) string Idle_Animation = "npc_cucco_idle";

	[ 
		EditType("Animation") 
	]
	public serialized(1) string Scared_Animation = "npc_cucco_scared";
	
	[ 
		EditType("Animation") 
	]
	public serialized(300) string Hurt_Animation = "npc_cucco_hurt";
	
	public serialized(1) bool Petted = false;
	public serialized(301) bool Has_Been_Angered = false;
	public serialized(302) bool Petted_Local = false;

	// Idle sounds.
	public float m_next_idle_sound = 0.0f;

	// Current state.
	public bool m_face_left = false;

	// Random movement.
	private float	m_last_movement_progress_time;
	private Vec3	m_last_movement_progress_position;
	
	private float	m_next_move_time;
	private float	m_next_turn_time;
	private Vec3	m_target_position;

	private Vec3	m_last_position;

	private float	m_damage_taken_this_frame;
	private float	m_damage_taken;
	private float	m_rage_cooloff;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float MIN_IDLE_SOUND_INTERVAL = 5.0f;
	const float MAX_IDLE_SOUND_INTERVAL = 15.0f;
	
	const float BASE_WALK_SPEED		= 7.0;	
	const float PANIC_WALK_SPEED	= 30.0f;

	const float IDLE_WALK_DISTANCE_MIN = 32.0f;
	const float IDLE_WALK_DISTANCE_MAX = 64.0f;

	const float IDLE_WALK_INTERVAL_MIN = 3000;
	const float IDLE_WALK_INTERVAL_MAX = 10000;

	const float IDLE_TURN_INTERVAL_MIN = 1000;
	const float IDLE_TURN_INTERVAL_MAX = 5000;
	
	const float PANIC_WALK_DISTANCE_MIN = 48.0f;
	const float PANIC_WALK_DISTANCE_MAX = 72.0f;

	const float PANIC_WALK_INTERVAL_MIN = 500;
	const float PANIC_WALK_INTERVAL_MAX = 2000;
	
	const float MOVEMENT_PROGRESS_CHECK_INTERVAL	= 0.5f;
	const float MOVEMENT_PROGRESS_CHECK_THRESHOLD	= 1.5f; 

	const float RAGE_DAMAGE_THRESHOLD = 300.0f;

	Cucco()
	{
		Bounding_Box = Vec4(0, 0, 29, 29);

		m_next_idle_sound = Math.Rand(MIN_IDLE_SOUND_INTERVAL, MAX_IDLE_SOUND_INTERVAL);
	}
	
	public override float Get_Duration(Pawn other)
	{
		return 100.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#pet_cucco_prompt");
	}
	
	public override bool Can_Interact(Pawn other)
	{
		if (Petted_Local)
		{
			return false;
		}

		if (Has_Been_Angered)
		{
			return false;
		}

		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			return true;
		}

		return false;
	}

	public override void Interact(Pawn other)
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			// Play save animation.
			sprite.Play_Oneshot_Animation(Scared_Animation, 1.0f);

			// Add score.
			FX.Spawn_Untracked("coin_splash_long_large", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, 1000);

			Audio.Play3D("sfx_objects_npc_cucco_pet", this.Center);
			
			Log.Write("Petted cucco " + this.Net_ID);

			Petted_Local = true;
		}
	}

	public void Update_Sprites()
	{		
		bool isIdle = (m_last_position == Position);

		if (this.Position.X < m_last_position.X)
		{
			m_face_left = true;
		}
		else if (this.Position.X > m_last_position.X)
		{			
			m_face_left = false;
		}

		sprite.Animation_Speed = isIdle ? 0.0f : 1.0f;
		sprite.Flip_Horizontal = !m_face_left;		

		m_last_position = this.Position;

		if (m_damage_taken_this_frame > 0.0f)
		{
			// Play save animation.
			sprite.Play_Oneshot_Animation(Hurt_Animation, 1.0f);

			// Spawn a hit effect.
			FX.Spawn_Untracked("human_hit", Vec3(this.World_Bounding_Box.Center(), 0.0f), 0.0f, Layer);
			
			// Spray effect.
			FX.Spawn_Untracked("damage_score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, <int>Math.Ceiling(m_damage_taken_this_frame));

			// Cluck cluck!
			Audio.Play3D("sfx_objects_npc_cucco_pet", this.Center);

			// Panic?
			if (Network.Is_Server())
			{
				// Rage time?
				m_damage_taken += m_damage_taken_this_frame;
				if (m_damage_taken > RAGE_DAMAGE_THRESHOLD && m_rage_cooloff < 0.0f)
				{
					m_damage_taken = 0.0f;
					m_rage_cooloff = 30.0f;

					Begin_Rage();
				}

				change_state(Panic);
			}
		}

		m_rage_cooloff -= Time.Get_Delta_Seconds();
		m_damage_taken_this_frame = 0.0f;
	}

	// -------------------------------------------------------------------------
	// Cluck Cluck Cluck
	// -------------------------------------------------------------------------
	default state Idle
	{
		event void On_Enter()
		{		
			sprite.Animation		= Idle_Animation;
			sprite.Animation_Speed	= 1.0f;

			m_next_move_time = Time.Get_Ticks() + Math.Rand(IDLE_WALK_INTERVAL_MIN, IDLE_WALK_INTERVAL_MAX);
			m_next_turn_time = Time.Get_Ticks() + Math.Rand(IDLE_TURN_INTERVAL_MIN, IDLE_TURN_INTERVAL_MAX);
		}
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				// Move periodically.
				if (Time.Get_Ticks() > m_next_move_time)
				{
					Vec3 rand_offset = Vec3(Vec2.Rand(IDLE_WALK_DISTANCE_MIN, IDLE_WALK_DISTANCE_MAX), 0.0f);
					m_target_position = this.Position + rand_offset;

					change_state(Idle_Walking);
				}

				// Occassionally look in different direction.
				if (Time.Get_Ticks() > m_next_turn_time)
				{
					m_face_left = (Math.Rand(0, 2) == 0);
					m_next_turn_time = Time.Get_Ticks() + Math.Rand(IDLE_TURN_INTERVAL_MIN, IDLE_TURN_INTERVAL_MAX);
				}
			}

			// Idle clucking.
			if (m_next_idle_sound <= 0.0f)
			{
				m_next_idle_sound = Math.Rand(MIN_IDLE_SOUND_INTERVAL, MAX_IDLE_SOUND_INTERVAL);
				Audio.Play3D("sfx_objects_npc_cucco_idle_cluck", this.Center);
			}
			else
			{
				m_next_idle_sound -= Time.Get_Delta_Seconds();
			}

			Update_Sprites();
		}
	}
	
	// -------------------------------------------------------------------------
	//  WHY ARE YOU HURTING ME :(
	// -------------------------------------------------------------------------
	state Panic
	{
		event void On_Enter()
		{
			Vec3 rand_offset = Vec3(Vec2.Rand(PANIC_WALK_DISTANCE_MIN, PANIC_WALK_DISTANCE_MAX), 0.0f);
			m_target_position = this.Position + rand_offset;
			m_next_move_time = Time.Get_Ticks() + Math.Rand(PANIC_WALK_INTERVAL_MIN, PANIC_WALK_INTERVAL_MAX);

			Has_Been_Angered = true;
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				bool new_movement = false;

				// Time for new movement?
				if (Time.Get_Ticks() > m_next_move_time)
				{
					new_movement = true;
				}

				// Move towards are random target.
				if (Move(m_target_position, PANIC_WALK_SPEED))
				{
					new_movement = true;
				}

				// New movement!
				if (new_movement == true)
				{
					Vec3 rand_offset = Vec3(Vec2.Rand(PANIC_WALK_DISTANCE_MIN, PANIC_WALK_DISTANCE_MAX), 0.0f);
					m_target_position = this.Position + rand_offset;
					m_next_move_time = Time.Get_Ticks() + Math.Rand(PANIC_WALK_INTERVAL_MIN, PANIC_WALK_INTERVAL_MAX);				
				}
			}

			Update_Sprites();
		}
	}   

	// -------------------------------------------------------------------------
	// Waling somewhere idley.
	// -------------------------------------------------------------------------
	state Idle_Walking
	{
		event void On_Enter()
		{
			m_last_movement_progress_time = 0.0f;
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				// Move towards are random target.
				if (Move(m_target_position, BASE_WALK_SPEED))
				{
					change_state(Idle);
				}

				// Check we are making progress, if not go idle.
				if (!Check_Movement_Progress())
				{
					change_state(Idle);
				}
			}
				
			Update_Sprites();
		}
	}   
	
	// -------------------------------------------------------------------------
	// Damage
	// -------------------------------------------------------------------------
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
            Item_Archetype weapon_archetype)
	{		
		Human_Player human_instigator = <Human_Player>instigator;
		if (!human_instigator)
		{
			return false;
		}

        if (damage <= 0.0f)
        { 
            return false;
        }
        
		if (type == CollisionDamageType.Buff_Ammo	||
			type == CollisionDamageType.Buff_Health	||
			type == CollisionDamageType.Buff_Damage	||
			type == CollisionDamageType.Buff_Price	||
			type == CollisionDamageType.Buff_Reload ||
			type == CollisionDamageType.Buff_ROF	||
			type == CollisionDamageType.Buff_Speed	||
			type == CollisionDamageType.Buff_XP)
		{
            return false;
		}
					
		m_damage_taken_this_frame += damage;

		return true;
	}

	// -------------------------------------------------------------------------
	// RAAAAAAAAAAAAAAAGE
	// -------------------------------------------------------------------------
	void Begin_Rage()
	{
		Log.Write("Cucco rage beginning!");

		rpc(RPCTarget.All, RPCTarget.None) RPC_Rage();
	}
	rpc void RPC_Rage()
	{		
		Log.Write("Spawning cucco's rage!");

		FX.Spawn_Untracked("cucco_rage", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, 0);
		Audio.Play3D("sfx_objects_npc_cucco_rage", this.Center);
	}

	// -------------------------------------------------------------------------
	// Movement functionality.
	// -------------------------------------------------------------------------
	bool Move(Vec3 target, float speed)
	{ 
		Vec3 start_position = this.Position;
		Vec3 final_position = start_position;
		float delta = Time.Get_Delta();
		bool at_target = false;

		// Apply movement.
		Vec3 vector_to_target = (target - final_position).Unit();
		Vec3 movement = ((vector_to_target * speed) * delta);
		final_position = final_position + movement;
		Vec3 vector_to_target_after = (target - final_position).Unit();

		// TODO: Should probably prevent movement overshooting target here.

		// If this movement will take us onto or past the target, then we are at the end of our movement.
		at_target = vector_to_target.Dot(vector_to_target_after) < 0.0 || vector_to_target_after == Vec3(0.0f);

		// Onwards!
		MoveTo(final_position);

		return at_target;
	}
	
	bool Check_Movement_Progress()
	{		
		// Not making any progress? Bah, probably a wall in the way,
		// lets just stop.
		if (m_last_movement_progress_time == 0)
		{
			m_last_movement_progress_position = this.Position;
		}

		m_last_movement_progress_time += Time.Get_Delta();

		if (m_last_movement_progress_time >= MOVEMENT_PROGRESS_CHECK_INTERVAL)
		{
			float delta_movement = (m_last_movement_progress_position - this.Position).Length();

			if (delta_movement < MOVEMENT_PROGRESS_CHECK_THRESHOLD)
			{
				return false;
			}

			m_last_movement_progress_time = 0.0f;
		}

		return true;
	}


	replicate(ReplicationPriority.High)
	{
		Position
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		} 
		Has_Been_Angered
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Animation	= Idle_Animation;
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
		}

		CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(5, 20, 18, 8);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Player;
			Collides_With	= CollisionGroup.Environment;
		}

		// Hit boxies.
		CollisionComponent body_hitbox
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(4, 3, 24, 25);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Player;
			Collides_With	= CollisionGroup.Damage;
		}
	}
}