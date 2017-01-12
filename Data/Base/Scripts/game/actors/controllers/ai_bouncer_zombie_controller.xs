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
using game.actors.enemies.bouncer_zombie;
using game.actors.decoration.movable.movable;

[
	Name("AI Bouncer Zombie Controller"), 
	Description("Controls bouncer zombies which charge up and pounce at the player.") 
]
public class AI_Bouncer_Zombie_Controller : Pawn_Controller
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
	
	private float		m_in_los_timer;

	private Vec3		m_jump_target;
	private float		m_jump_duration;
	private float		m_jump_elapsed;
	private Vec3		m_vec_to_jump_target;

	private float		m_charge_timer;

	private serialized(1) float m_walk_speed_variance = 0.0f;

    private bool        m_safe_to_change_controller = true;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float BASE_WALK_SPEED			= 3.0;	
	const float WALK_SPEED_VARIANCE		= 1.0;	

	const float	BASE_JUMP_SPEED			= 30.0f;
	const float MAX_JUMP_DISTANCE		= 72.0f;
	const float MIN_JUMP_DURATION		= 1.0f;
	const float MAX_JUMP_DURATION       = 3.0f;

	const float IDLE_TURN_INTERVAL_MIN = 1000;
	const float IDLE_TURN_INTERVAL_MAX = 5000;
	
	const float TARGET_SWITCH_DISTANCE_BIAS = 1.5f;

	const float ATTACK_INTERVAL			= 2.0f;
	const float ATTACK_DISTANCE			= 24.0f;
	
		  float DESTROYABLE_FOV			= Math.PI * 1.5f;

	const float SCAN_FOR_TARGETS_INTERVAL	= 2.0f;

	const float LOS_CHARGE_DELAY		= 1.0f;

	const float CHARGE_DURATION			= 1.0f;
	
	AI_Bouncer_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);
	}
    
    public override bool Is_Safe_To_Switch_Controller()
    {
        return m_safe_to_change_controller;
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
				Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;
			
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
	// Chasing after the closest player.
	// -------------------------------------------------------------------------
	state Chasing
	{
		event void On_Enter()
		{
			Store_Last_State();

			m_last_movement_progress_time = 0.0f;
			m_in_los_timer = 0.0f;
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;
				Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;
				float delta = Time.Get_Delta();
				
				// Target despawned?
				if (m_chase_target == null || 
					!Scene.Is_Spawned(m_chase_target) ||
					!m_chase_target.Can_Target())
				{
					change_state(Idle);
					return;
				}

				// If we can see the player, charge up a jump!
				if (path_planner.Target_In_LOS)
				{
					m_in_los_timer += Time.Get_Delta_Seconds();
					if (m_in_los_timer > LOS_CHARGE_DELAY && !pawn.IsParalyzed())
					{
						change_state(Charging);
						return;
					}
				}
				else
				{
					m_in_los_timer = 0.0f;
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
	// Chargin up a jump!
	// -------------------------------------------------------------------------
	state Charging
	{
		event void On_Enter()
		{
			Store_Last_State();
			m_charge_timer = 0.0f;
            m_safe_to_change_controller = false;

			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Begin_Charge();
			}
		}		
		event void On_Exit()
		{
            m_safe_to_change_controller = true;

			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Finish_Charge();
			}
		}
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				float delta = Time.Get_Delta();
				Pawn pawn = Possessed;
				Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;

				// Target despawned?
				if (m_chase_target == null ||
					!Scene.Is_Spawned(m_chase_target) ||
					!m_chase_target.Can_Target())
				{
					change_state(Idle);
					return;
				}
				
				// Move to attacking?
				if (!path_planner.Target_In_LOS)
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

				// Time to jump yet?
				m_charge_timer += Time.Get_Delta_Seconds();
				if (m_charge_timer > CHARGE_DURATION && !pawn.IsParalyzed())
				{
					m_jump_target = m_chase_target.Collision_Center;
					change_state(Jumping);
				}
			}

			Track_Pawn();
			Store_Last_State();
		}
	}   
	
	// -------------------------------------------------------------------------
	//  Jumping
	// -------------------------------------------------------------------------
	state Jumping
	{
		event void On_Enter()
		{
			Store_Last_State();
            m_safe_to_change_controller = false;
			
			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Begin_Jump();
				
				Vec3 vec_to_target = (m_jump_target - zombie.Collision_Center);
				float distance = vec_to_target.Length();
				if (distance > MAX_JUMP_DISTANCE)
				{
					m_jump_target = zombie.Collision_Center + (vec_to_target.Unit() * MAX_JUMP_DISTANCE);
					distance = MAX_JUMP_DISTANCE;
				}

				m_vec_to_jump_target = vec_to_target;

				m_jump_duration = Math.Min(MAX_JUMP_DURATION, Math.Max(MIN_JUMP_DURATION, (distance * 0.3f) / BASE_JUMP_SPEED));
				m_jump_elapsed = 0.0f;
			}
		}		
		event void On_Exit()
		{
            m_safe_to_change_controller = true;

			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Finish_Jump();
			}
		}
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				float delta = Time.Get_Delta_Seconds();
				Pawn pawn = Possessed;
				Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;

				Vec3 offset = zombie.Collision_Center - zombie.Position;

				Move_Towards(zombie.Position + (m_vec_to_jump_target), BASE_JUMP_SPEED, true, m_old_direction);

				float elapsed_delta = (m_jump_elapsed / m_jump_duration);
				zombie.Update_Jump_Delta(elapsed_delta);

				m_jump_elapsed += delta;
				if (m_jump_elapsed >= m_jump_duration)
				{
					change_state(Idle);
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
			m_chase_target = closest_pawn;
			change_state(Chasing);

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
		// Can we attack?
		m_attack_timer -= Time.Get_Delta();
		if (m_attack_timer > 0.0f)
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

		// If yes then jump on the destroyable.
		if (potential_targets.Length() > 0)
		{        
			m_jump_target = Vec3(potential_targets[0].World_Bounding_Box.Center(), 0.0f);
			change_state(Jumping);				
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
