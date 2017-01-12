  // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.time;
using system.components.collision;
using system.events;
using game.actors.enemies.fodder_zombie;

// TODO: LOS check when deciding if we should chase something.

// Handy switchs for keeping zombies still when testing stuff.
//#define OPT_FODDER_ZOMBIE_MOVE_RANDOMLY 1
//#define OPT_FODDER_ZOMBIE_TURN_RANDOMLY 1

[
	Name("AI Fodder Zombie Controller"), 
	Description("Controls fodder zombies using some simple AI.") 
]
public class AI_Fodder_Zombie_Controller : Pawn_Controller
{
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	private Direction 	m_old_direction					= Direction.S;
	private Direction 	m_direction						= Direction.S;

	private float		m_next_move_time;
	private float		m_next_turn_time;
	private Vec3		m_target_position;

	private bool		m_disturbed;
	private Actor		m_disturb_instigator;

	private float		m_last_movement_progress_time;
	private Vec3		m_last_movement_progress_position;

	private float		m_attack_timer;

	private float		m_target_scan_timer;
	
	private serialized(1) float m_walk_speed_variance = 0.0f;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float BASE_WALK_SPEED		= 7.0;	
	const float WALK_SPEED_VARIANCE	= 3.0;	
	const float BASE_CHASE_SPEED	= 20.0;	
	const float PANIC_WALK_SPEED	= 30.0f;

	const float IDLE_WALK_DISTANCE_MIN = 16.0f;
	const float IDLE_WALK_DISTANCE_MAX = 32.0f;

	const float IDLE_WALK_INTERVAL_MIN = 3000;
	const float IDLE_WALK_INTERVAL_MAX = 10000;

	const float IDLE_TURN_INTERVAL_MIN = 1000;
	const float IDLE_TURN_INTERVAL_MAX = 5000;
	
	const float PANIC_WALK_DISTANCE_MIN = 32.0f;
	const float PANIC_WALK_DISTANCE_MAX = 64.0f;

	const float PANIC_WALK_INTERVAL_MIN = 500;
	const float PANIC_WALK_INTERVAL_MAX = 2000;

		  float FIELD_OF_VIEW			= Math.HalfPI; // 45o should be enough?
	const float VIEW_DISTANCE			= 164.0f;
	const float DANGER_DISTANCE			= 52.0f;

	const float MOVEMENT_PROGRESS_CHECK_INTERVAL	= 0.5f;
	const float MOVEMENT_PROGRESS_CHECK_THRESHOLD	= 2.2f; 

	const float ATTACK_INTERVAL			= 2.0f;
	const float ATTACK_DISTANCE			= 24.0f;

	const float SCAN_FOR_TARGETS_INTERVAL	= 0.5f;
	
	AI_Fodder_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);
	}

	// -------------------------------------------------------------------------
	// This state just waits for a pawn to control.
	// -------------------------------------------------------------------------
	default state Waiting_For_Possession
	{
	}
	 
	// -------------------------------------------------------------------------
	// Idle state - just stand around waiting to be alerted to a target.
	// -------------------------------------------------------------------------
	state Idle
	{
		event void On_Enter()
		{
			Store_Last_State();
			Track_Pawn();

			m_next_move_time = Time.Get_Ticks() + Math.Rand(IDLE_WALK_INTERVAL_MIN, IDLE_WALK_INTERVAL_MAX);
			m_next_turn_time = Time.Get_Ticks() + Math.Rand(IDLE_TURN_INTERVAL_MIN, IDLE_TURN_INTERVAL_MAX);
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;
				Fodder_Zombie zombie = <Fodder_Zombie>pawn;

				// Pawn now decapitated?
				if (zombie.Is_Decapitated == true)
				{
					change_state(Panic);
					return;
				}

				// Move periodically.
//#if OPT_FODDER_ZOMBIE_MOVE_RANDOMLY
				if (Time.Get_Ticks() > m_next_move_time)
				{
					Vec3 rand_offset = Vec3(Vec2.Rand(IDLE_WALK_DISTANCE_MIN, IDLE_WALK_DISTANCE_MAX), 0.0f);
					m_target_position = pawn.Position + rand_offset;

					change_state(Idle_Walking);
				}
//#endif

				// Occassionally look in different direction.
//#if OPT_FODDER_ZOMBIE_TURN_RANDOMLY
				if (Time.Get_Ticks() > m_next_turn_time)
				{
					pawn.Turn_Towards(Math.Rand(0, Direction.COUNT));

					m_next_turn_time = Time.Get_Ticks() + Math.Rand(IDLE_TURN_INTERVAL_MIN, IDLE_TURN_INTERVAL_MAX);
				}
//#endif

				// Scan for targets.
				Scan_For_Targets(false);
			}	

			Track_Pawn();
			Store_Last_State();
		}
		event void On_Hear_Sound(Event_Hear_Sound evt)
		{
			m_disturbed = true;
			m_disturb_instigator = evt.Instigator;
		}
	}   
	
	// -------------------------------------------------------------------------
	// Panic state - Run around like crazy, probably be decapitated.
	// -------------------------------------------------------------------------
	state Panic
	{
		event void On_Enter()
		{
			Pawn pawn = Possessed;

			Store_Last_State();

			Vec3 rand_offset = Vec3(Vec2.Rand(PANIC_WALK_DISTANCE_MIN, PANIC_WALK_DISTANCE_MAX), 0.0f);
			m_target_position = pawn.Position + rand_offset;
			m_next_move_time = Time.Get_Ticks() + Math.Rand(PANIC_WALK_INTERVAL_MIN, PANIC_WALK_INTERVAL_MAX);
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;

				bool new_movement = false;

				// Time for new movement?
				if (Time.Get_Ticks() > m_next_move_time)
				{
					new_movement = true;
				}

				// Move towards are random target.
				if (Move_Towards(m_target_position, Get_Panic_Speed(), true, m_old_direction))
				{
					new_movement = true;
				}

				// New movement!
				if (new_movement == true)
				{
					Vec3 rand_offset = Vec3(Vec2.Rand(PANIC_WALK_DISTANCE_MIN, PANIC_WALK_DISTANCE_MAX), 0.0f);
					m_target_position = pawn.Position + rand_offset;
					m_next_move_time = Time.Get_Ticks() + Math.Rand(PANIC_WALK_INTERVAL_MIN, PANIC_WALK_INTERVAL_MAX);				
				}
			}
							
			Track_Pawn();
			Store_Last_State();
		}
	}   

	// -------------------------------------------------------------------------
	// Idle walking state - idle'y walking to a target position.
	// -------------------------------------------------------------------------
	state Idle_Walking
	{
		event void On_Enter()
		{
			Store_Last_State();

			m_last_movement_progress_time = 0.0f;
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;
				Fodder_Zombie zombie = <Fodder_Zombie>pawn;
				
				// Pawn now decapitated?
				if (zombie.Is_Decapitated == true)
				{
					change_state(Panic);
					return;
				}

				// Move towards are random target.
				if (Move_Towards(m_target_position, Get_Walk_Speed(), true, m_old_direction))
				{
					change_state(Idle);
				}

				// Check we are making progress, if not go idle.
				if (!Check_Movement_Progress())
				{
					change_state(Idle);
				}

				// Scan for targets.
				Scan_For_Targets(true);
			}
				
			Track_Pawn();
			Store_Last_State();
		}
		event void On_Hear_Sound(Event_Hear_Sound evt)
		{
			m_disturbed = true;
			m_disturb_instigator = evt.Instigator;
		}
	}   
	
	// -------------------------------------------------------------------------
	// Attacking state - keeps using weapon until 
	// -------------------------------------------------------------------------
	state Attacking
	{
		event void On_Enter()
		{
			Store_Last_State();
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				float delta = Time.Get_Delta();
				Pawn pawn = Possessed;
				Fodder_Zombie zombie = <Fodder_Zombie>pawn;
				
				// Pawn now decapitated?
				if (zombie.Is_Decapitated == true)
				{
					change_state(Panic);
					return;
				}

				// Face closest target.
				if (!Face_Closest_Target())
				{
					change_state(Idle);
				}

				// Fire the pawns weapon (slash).	
				m_attack_timer -= delta;

				if (m_attack_timer <= 0.0f && !pawn.IsParalyzed())
				{
					pawn.Begin_Fire();
					pawn.Finish_Fire();
				
					m_attack_timer = ATTACK_INTERVAL;
				}
			}
				
			Track_Pawn();
			Store_Last_State();
		}
	}   
	
	// -------------------------------------------------------------------------
	//  Basic targetting code.
	// -------------------------------------------------------------------------
	void Scan_For_Targets(bool always_run)
	{
		float delta_t = Time.Get_Delta_Seconds();

		Pawn pawn = Possessed;
		Vec3 pawn_center = pawn.Center;
		Vec4 bb = pawn.Bounding_Box;

		// Pawn was disturbed by something, go investigate.
		if (m_disturbed && m_disturb_instigator != null)
		{
			m_target_position = m_disturb_instigator.Center - Vec3(bb.Width * 0.5f, bb.Height * 0.5f, 0.0f);
			m_last_movement_progress_time = 0.0f;
 			change_state(Idle_Walking);
		}

		// Only do visual scans periodically, to expensive to do each frame.
		if (!always_run)
		{
			m_target_scan_timer -= delta_t;
			if (m_target_scan_timer > 0.0f)
			{
				return;
			}
			m_target_scan_timer = SCAN_FOR_TARGETS_INTERVAL;
		}

		// Otherwise scan for nearby targets visually.
		Vec3 heading = Direction_Helper.Vectors[pawn.Current_Direction];

		// Gets actors inside our FOV.
		Actor[] potential_targets = Scene.Find_Actors_In_FOV(
			typeof(Human_Player), 
			VIEW_DISTANCE,
			pawn_center, 
			FIELD_OF_VIEW, 
			heading
		);
		
		// Get potential actors inside our danger zone where we will always respond.
		Actor[] potential_danger_targets = Scene.Find_Actors_In_Radius(
			typeof(Human_Player), 
			DANGER_DISTANCE,
			pawn_center
		);

		// Merge arrays.
		potential_targets.AddLast(potential_danger_targets);

		float closest_distance = 99999.0f;
		Pawn closest_pawn = null;

		foreach (Pawn target in potential_targets)
		{
			if (target != pawn)
			{
				if (target.Can_Target())
				{
					if (target.Team_Index != pawn.Team_Index)
					{
						// If close enough attack, otherwise wonder towards them.
						float distance = (target.Center - Center).Length();

						if (distance < closest_distance)
						{
							closest_distance = distance;
							closest_pawn = target;
						}
					}
				}
			}
		}

		if (closest_pawn != null)
		{
			if (closest_distance <= ATTACK_DISTANCE)
			{
				// Get attacking.
				change_state(Attacking);
			}
			else
			{
				m_target_position = closest_pawn.Center - Vec3(bb.Width * 0.5f, bb.Height * 0.5f, 0.0f);
				change_state(Idle_Walking);
			}
		}

		m_disturbed = false;
	}

	bool Face_Closest_Target()
	{
		Pawn pawn = Possessed;
		Vec3 pawn_center = pawn.Center;
		Vec4 bb = pawn.Bounding_Box;

		Actor[] potential_targets = Scene.Find_Actors_In_Radius(
			typeof(Pawn), 
			ATTACK_DISTANCE * 1.3f,
			pawn_center
		);

		float closest_distance = 99999.0f;
		Pawn closest_pawn = null;

		foreach (Pawn target in potential_targets)
		{
			if (target != pawn)
			{
				if (target.Can_Target())
				{
					if (target.Team_Index != pawn.Team_Index)
					{
						// If close enough attack, otherwise wonder towards them.
						float distance = (target.Center - Center).Length();

						if (distance < closest_distance)
						{
							closest_distance = distance;
							closest_pawn = target;
						}
					}
				}
			}
		}

		if (closest_pawn != null)
		{
			Vec3 delta = (closest_pawn.Center - Center);

			Direction new_dir = Direction_Helper.Calculate_Direction(delta.X, delta.Y, m_old_direction);
			pawn.Turn_Towards(new_dir);

			return true;
		}

		return false;
	}
	
	// -------------------------------------------------------------------------
	//  Checks we are making movement to target.
	// -------------------------------------------------------------------------
	bool Check_Movement_Progress()
	{		
		Pawn pawn = Possessed;

		// Not making any progress? Bah, probably a wall in the way,
		// lets just stop.
		if (m_last_movement_progress_time == 0)
		{
			m_last_movement_progress_position = pawn.Position;
		}

		m_last_movement_progress_time += Time.Get_Delta();

		if (m_last_movement_progress_time >= MOVEMENT_PROGRESS_CHECK_INTERVAL)
		{
			float delta_movement = (m_last_movement_progress_position - pawn.Position).Length();

			if (delta_movement < MOVEMENT_PROGRESS_CHECK_THRESHOLD)
			{
				return false;
			}

			m_last_movement_progress_time = 0.0f;
		}

		return true;
	}

	// -------------------------------------------------------------------------
	//  Basic movement code.
	// -------------------------------------------------------------------------
	float Get_Walk_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return (BASE_WALK_SPEED + m_walk_speed_variance) * speed_modifier; // Note: Fodder zombies don't speed up with difficulty - yet, seems better not to.
	}             
	float Get_Panic_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return PANIC_WALK_SPEED * speed_modifier; 
	}
	
	void Store_Last_State()
	{	
		m_old_direction = m_direction;
	}

	void Track_Pawn()
	{
		Position = this.Possessed.Position;
	}
	 
	// -------------------------------------------------------------------------
	// General functions.
	// -------------------------------------------------------------------------
	override void On_Possession_Changed(Pawn other)
	{	
		if (other != null)
		{
			change_state(Idle);
		}
		else
		{
			Log.Write("Lost possession of pawn, despawning controller.");
			Scene.Despawn(this);
		}
	}	

	override void On_Possessed_Despawned()
	{
		Log.Write("Pawn despawned, despawning controller.");
		Scene.Despawn(this);
	}

}