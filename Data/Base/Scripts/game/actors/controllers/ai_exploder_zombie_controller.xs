 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using system.time;
using system.components.collision;
using system.components.path_planner;
using system.events;
using game.actors.enemies.exploder_zombie;
using game.difficulty.difficulty_manager;

[
	Name("AI Exploder Zombie Controller"), 
	Description("Controls exploder zombies using some simple run-at-the-closest-player AI.") 
]
public class AI_Exploder_Zombie_Controller : Pawn_Controller
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

	private float		m_explode_timer;
	private float		m_explode_flash_timer;

	private float		m_target_scan_timer;
	
	private serialized(1) float m_walk_speed_variance = 0.0f;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float BASE_WALK_SPEED			= 6.0;	
	const float WALK_SPEED_VARIANCE		= 3.0;	
	
    const float DESTROYABLE_ATTACK_DAMAGE   = 10.0f;

	const float IDLE_TURN_INTERVAL_MIN = 1000;
	const float IDLE_TURN_INTERVAL_MAX = 5000;
	
	const float TARGET_SWITCH_DISTANCE_BIAS = 1.5f;

	const float ATTACK_INTERVAL			= 2.0f;
	const float ATTACK_DISTANCE			= 24.0f;
	
		  float DESTROYABLE_FOV			= Math.PI * 1.5f;

	const float EXPLODE_RADIUS			= 70.0f;
	const float	EXPLODE_MAX_TIMER		= 8.0f;
	const float MAX_FLASH_INTERVAL		= 800.0f;
	const float MIN_FLASH_INTERVAL		= 150.0f; // Careful with this, an rpc is fired each time.

	const float SCAN_FOR_TARGETS_INTERVAL	= 2.0f;

	AI_Exploder_Zombie_Controller()
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
				Exploder_Zombie zombie = <Exploder_Zombie>pawn;
				
				// Find a target.
				if (Find_Target())
				{
					return;
				}
				
				// Occassionally look in different direction.
				if (Time.Get_Ticks() > m_next_turn_time)
				{
					pawn.Turn_Towards(Direction_Helper.Rand4Dir());
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
		}		
		event void On_Tick()
		{
			if (Network.Is_Server())
			{
				Pawn pawn = Possessed;
				Exploder_Zombie zombie = <Exploder_Zombie>pawn;
				float delta = Time.Get_Delta();
				
				// Target despawned?
				if (m_chase_target == null || 
					!Scene.Is_Spawned(m_chase_target) || 
					!m_chase_target.Can_Target())
				{
					change_state(Idle);
					return;
				}

				// Plan a path and move towards the target.
				path_planner.Source_Position = zombie.Collision_Center;
				path_planner.Target_Position = m_chase_target.Collision_Center;

                Vec3 chase_vector = path_planner.Movement_Vector;
			
				// Move towards the chase vector.
				Move_Towards(Position + chase_vector, Get_Walk_Speed(), true, m_old_direction, true);
				
				// Check if anything destroyable is in our way.
				Check_For_Destroyables(chase_vector);

				// Scan for targets incase one is more to our liking.
				if (Find_Target())
				{
					return;
				}
		
				// Check explosion!
				Check_Explode();
			}

			Track_Pawn();
			Store_Last_State();
		}
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
            // Drawin the destroyables health, we cannot do anymore as we don't want to use our primary attack.
            foreach (Movable obj in potential_targets)
            {
                obj.Reduce_Health(DESTROYABLE_ATTACK_DAMAGE);
            }
            foreach (Enemy_Barricade obj in potential_targets)
            {
                obj.Reduce_Health(DESTROYABLE_ATTACK_DAMAGE);
            }

			m_attack_timer = ATTACK_INTERVAL;
		}
	}

	// -------------------------------------------------------------------------
	//  Explodes after a little while of being close to the user.
	// -------------------------------------------------------------------------
	bool Check_Explode()
	{
		Pawn pawn = Possessed;
		Exploder_Zombie zombie = <Exploder_Zombie>pawn;

		float distance = (m_chase_target.Center - Center).Length();

		if (pawn.IsParalyzed())
		{
			return false;
		}
		
		// Charge up based on distance.
		if (distance <= EXPLODE_RADIUS)
		{
			m_explode_timer += Time.Get_Delta();
		}
		else if (distance > EXPLODE_RADIUS)
		{
			m_explode_timer = Math.Max(0.0f, m_explode_timer - Time.Get_Delta());
		}
		
		// Splode time.
		if (m_explode_timer > 0)
		{
			float ticks = Time.Get_Ticks();
			float interval = Math.Max(MIN_FLASH_INTERVAL, (1.0f - (m_explode_timer / EXPLODE_MAX_TIMER)) * MAX_FLASH_INTERVAL);

			if (ticks - m_explode_flash_timer > interval)
			{
				bool critical = (m_explode_timer > EXPLODE_MAX_TIMER * 0.5f);
				
				rpc(RPCTarget.All, RPCTarget.None) RPC_Explosion_Warning(critical);

				m_explode_flash_timer = ticks;
			}
		}

		// Time to die?
		if (Network.Is_Server())
		{
			if (m_explode_timer > EXPLODE_MAX_TIMER)
			{
				zombie.Explode(null);
			}
		}
	}

	private rpc void RPC_Explosion_Warning(bool critical)
	{
		Pawn pawn = Possessed;
		Exploder_Zombie zombie = <Exploder_Zombie>pawn;
		
		zombie.Explosion_Warning(critical);

		if (critical == true)
		{
			Audio.Play2D("sfx_objects_enemies_exploder_countdown");
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

		Direction new_dir = Direction_Helper.Calculate_Direction(delta.X, delta.Y, m_old_direction, true);
		pawn.Turn_Towards(new_dir);
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
