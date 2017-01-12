// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.time;
using system.components.collision;
using system.components.path_planner;
using system.events;
using game.difficulty.difficulty_manager;
using game.actors.enemies.chaser_zombie;
using game.actors.decoration.movable.movable;

[
	Name("AI Chaser Zombie Controller"), 
	Description("Controls chaser zombies using some simple run-at-the-closest-player AI.") 
]
public class AI_Chaser_Zombie_Controller : Pawn_Controller
{
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	private Direction 	m_old_direction					= Direction.S;
	private Direction 	m_direction						= Direction.S;

	private float		m_last_movement_progress_time;
	private Vec3		m_last_movement_progress_position;

	private float		m_attack_timer;
	private float		m_next_turn_time;

	private Pawn		m_chase_target;

	private float		m_next_move_time;
	private Vec3		m_target_position;

	private float		m_target_scan_timer;

	private serialized(1) float m_walk_speed_variance = 0.0f;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float BASE_WALK_SPEED			= 10.0;	
	const float WALK_SPEED_VARIANCE		= 5.0;	
	const float FAST_ZOMBIE_VARIANCE	= 10.0f;
	const float FAST_ZOMBIE_PROBABILITY	= 0.1;	
	const float PANIC_WALK_SPEED		= 30.0f;
	
	const float PANIC_WALK_DISTANCE_MIN = 32.0f;
	const float PANIC_WALK_DISTANCE_MAX = 64.0f;

	const float PANIC_WALK_INTERVAL_MIN = 500;
	const float PANIC_WALK_INTERVAL_MAX = 2000;
	
	const float IDLE_TURN_INTERVAL_MIN = 1000;
	const float IDLE_TURN_INTERVAL_MAX = 5000;
	
	const float TARGET_SWITCH_DISTANCE_BIAS = 1.5f;

	const float ATTACK_INTERVAL			= 2.0f;
	const float ATTACK_DISTANCE			= 24.0f;
	
		  float DESTROYABLE_FOV			= Math.PI * 1.5f;

	const float SCAN_FOR_TARGETS_INTERVAL	= 2.0f;
	
	AI_Chaser_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);

		if (Math.Rand(0.0f, 1.0f) <= FAST_ZOMBIE_PROBABILITY)
		{
			m_walk_speed_variance = FAST_ZOMBIE_VARIANCE;
		}
	}

	// -------------------------------------------------------------------------
	// This state just waits for a pawn to control.
	// -------------------------------------------------------------------------
	default state Waiting_For_Possession
	{
	}
	 
	// -------------------------------------------------------------------------
	// Idle state - stand still waiting for target.
	// -------------------------------------------------------------------------
	state Idle
	{
		event void On_Enter()
		{
			Store_Last_State();
			Track_Pawn();

			m_chase_target = null;
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;
				Chaser_Zombie zombie = <Chaser_Zombie>pawn;
			
				// Still crawling from the earth, wait.
				if (!zombie.Has_Crawled_From_Earth)
				{
					return;
				}

				// Find a target.
				if (Find_Target())
				{
					return;
				}
				
				// Occassionally look in different direction.
				if (Time.Get_Ticks() > m_next_turn_time)
				{
					pawn.Turn_Towards(Math.Rand(0, Direction.COUNT));

					m_next_turn_time = Time.Get_Ticks() + Math.Rand(IDLE_TURN_INTERVAL_MIN, IDLE_TURN_INTERVAL_MAX);
				}
			}	

			Track_Pawn();
			Store_Last_State();
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
	// Chasing after the closest player.
	// -------------------------------------------------------------------------
	state Chasing
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
				Chaser_Zombie zombie = <Chaser_Zombie>pawn;
				float delta = Time.Get_Delta();
				
				// Target despawned?
				if (m_chase_target == null || 
					!Scene.Is_Spawned(m_chase_target) ||
					!m_chase_target.Can_Target())
				{
					change_state(Idle);
					return;
				}

				// Pawn now decapitated?
				if (zombie.Is_Decapitated == true)
				{
					change_state(Panic);
					return;
				}

				// Move to attacking?
				float chase_distance = (m_chase_target.Center - Center).Length();
				if (chase_distance <= ATTACK_DISTANCE)
				{
					change_state(Attacking);
					return;
				}

				// Plan a path and move towards the target.
				path_planner.Source_Position = zombie.Collision_Center;
				path_planner.Target_Position = m_chase_target.Collision_Center;

                Vec3 chase_vector = path_planner.Movement_Vector;
			
				// Move towards the chase vector.
				Move_Towards(Position + chase_vector, Get_Walk_Speed(), true, m_old_direction);
				
				// Check if anything destroyable is in our way.
				Check_For_Destroyables(chase_vector);

				// Scan for targets incase one is more to our liking.
				if (Find_Target())
				{
					return;
				}
			}

			Track_Pawn();
			Store_Last_State();
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
				Chaser_Zombie zombie = <Chaser_Zombie>pawn;
				
				// Target despawned?
				if (m_chase_target == null ||
					!Scene.Is_Spawned(m_chase_target) ||
					!m_chase_target.Can_Target())
				{
					change_state(Idle);
					return;
				}

				// Pawn now decapitated?
				if (zombie.Is_Decapitated == true)
				{
					change_state(Panic);
					return;
				}
				
				// Move to attacking?
				float chase_distance = (m_chase_target.Center - Center).Length();
				if (chase_distance > ATTACK_DISTANCE)
				{
					change_state(Chasing);
					return;
				}

				// Find better targets if any available.
				if (Find_Target())
				{
					return;
				}

				// Face closest target.
				Face_Target(m_chase_target);

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
	bool Find_Target()
	{
		float delta_t = Time.Get_Delta_Seconds();

		// Only do visual scans periodically, to expensive to do each frame.
		m_target_scan_timer -= delta_t;
		if (m_target_scan_timer > 0.0f)
		{
			return false;
		}
		m_target_scan_timer = SCAN_FOR_TARGETS_INTERVAL;

		// Look for closest target.
		Pawn pawn = Possessed;
		Vec3 pawn_center = pawn.Center;
		Vec4 bb = pawn.Bounding_Box;

		Actor[] potential_targets = null;
		Enemy pawn_enemy = <Enemy>Possessed;
		if (pawn_enemy.Parent_Arena != null)
		{
			potential_targets = Scene.Find_Actors_With_Collision_In_Area(typeof(Human_Player), pawn_enemy.Parent_Arena.World_Bounding_Box);
		}
		else
		{
			potential_targets = Scene.Find_Actors(typeof(Human_Player));
		}

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

		// If we already have a target this target has to be considerably
		// better for us to consider the change.
		if (closest_pawn != null)
		{
			if (m_chase_target != null)
			{ 
				float chase_distance = (m_chase_target.Center - Center).Length();
				if (closest_distance * TARGET_SWITCH_DISTANCE_BIAS >= chase_distance)
				{
					closest_pawn = null;
				}
			}
		}

		// Otherwise lets go!
		if (closest_pawn != null)
		{
			if (closest_distance <= ATTACK_DISTANCE)
			{
				// Get attacking.
				change_state(Attacking);
			}
			else
			{
				m_chase_target = closest_pawn;
				change_state(Chasing);
			}

			return true;
		}

		return false;
	}

	bool Face_Target(Actor target)
	{
		Pawn pawn = Possessed;
		Vec3 delta = (target.Center - Center);

		Direction new_dir = Direction_Helper.Calculate_Direction(delta.X, delta.Y, m_old_direction);
		pawn.Turn_Towards(new_dir);
	}

	// -------------------------------------------------------------------------
	//  Checks we are making movement to target.
	// -------------------------------------------------------------------------
	void Check_For_Destroyables(Vec3 heading)
	{		
		Pawn pawn = Possessed;

		// Can we attack?
		m_attack_timer -= Time.Get_Delta();
		if (m_attack_timer > 0.0f || pawn.IsParalyzed())
		{
			return;
		}

		// Anything in our way to attack?
		Actor[] potential_targets = Scene.Find_Actors_In_FOV(
			typeof(EnemyAttackable), 
			ATTACK_DISTANCE * 1.5f,
			this.Collision_Center, 
			DESTROYABLE_FOV, 
			heading
		);

		// If yes then atttttttttack!
		if (potential_targets.Length() > 0)
		{
			pawn.Begin_Fire();
			pawn.Finish_Fire();
				
			m_attack_timer = ATTACK_INTERVAL;
		}
	}

	// -------------------------------------------------------------------------
	//  Basic movement code.
	// -------------------------------------------------------------------------
	float Get_Walk_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return (BASE_WALK_SPEED + m_walk_speed_variance) * Difficulty_Manager.Enemy_Speed_Multiplier * speed_modifier;
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
	
	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{
		PathPlannerComponent path_planner
		{
			Regenerate_Delta = 64.0f;
			Collision_Group  = CollisionGroup.Enemy;
		}
	}
}
