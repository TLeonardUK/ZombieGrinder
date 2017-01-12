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
	Name("Player Bouncer Zombie Controller"), 
	Description("Allows players to control a bouncer zombie!") 
]
public class Player_Bouncer_Zombie_Controller : Player_Generic_Zombie_Controller
{
	private serialized(1) float m_walk_speed_variance = 0.0f;
    	
	const float BASE_WALK_SPEED			= 3.0;	
	const float WALK_SPEED_VARIANCE		= 1.0;	
	const float HUMAN_SPEED_MULTIPLIER	= 1.25;	
    
	const float	BASE_JUMP_SPEED			= 30.0f;
	const float MAX_JUMP_DISTANCE		= 72.0f;
	const float MIN_JUMP_DURATION		= 1.0f;
    const float MAX_JUMP_DURATION       = 5.0f;

	const float CHARGE_DURATION			= 1.0f;

    const float PER_CHARGE_STAMINA      = 50.0f;

	private Vec3		m_jump_target;
	private float		m_jump_duration;
	private float		m_jump_elapsed;
	private Vec3		m_vec_to_jump_target;
    
	private float		m_charge_timer;

	private Vec3		m_movement_delta;

    private float       m_old_stamina;
    private float       m_new_stamina;

	Player_Bouncer_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
		m_walk_speed_variance = Math.Rand(0.0f, WALK_SPEED_VARIANCE);
	}

	override float Get_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return (BASE_WALK_SPEED + m_walk_speed_variance) * HUMAN_SPEED_MULTIPLIER * Difficulty_Manager.Enemy_Speed_Multiplier * speed_modifier;
	}           
    
	// -------------------------------------------------------------------------
	// Idle tick!
	// -------------------------------------------------------------------------
    override void Idle_Tick()
    {
		NetUser player = Owner;
            
		if (player.Is_Local)
		{
			Pawn pawn = Possessed;
            
            if (m_fire_down && pawn.Stamina > PER_CHARGE_STAMINA)
            {
                m_old_stamina = pawn.Stamina;
                m_new_stamina = pawn.Stamina - PER_CHARGE_STAMINA;
                change_state(Charging);
            }
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

			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Begin_Charge();
			}
		}		
		event void On_Exit()
		{
			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Finish_Charge();
			}
		}
		event void On_Tick()
		{
			float delta = Time.Get_Delta();
			Pawn pawn = Possessed;
			Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;

			// Time to jump yet?
			m_charge_timer += Time.Get_Delta_Seconds();

            float charge_delta = m_charge_timer / CHARGE_DURATION;
            pawn.Stamina = Math.Lerp(m_old_stamina, m_new_stamina, charge_delta);

			if (m_charge_timer > CHARGE_DURATION && !pawn.IsParalyzed())
			{
				m_jump_target = pawn.Collision_Center + (Direction_Helper.Vectors[pawn.Current_Direction] * MAX_JUMP_DISTANCE);
				change_state(Jumping);
                return;
			}

			NetUser player = Owner;            
			Update_Input();

            if (!m_fire_down)
            {
                change_state(Idle);
                return;
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
			Bouncer_Zombie zombie = <Bouncer_Zombie>Possessed;
			if (zombie)
			{
				zombie.Finish_Jump();
			}
		}
		event void On_Tick()
		{	
			float delta = Time.Get_Delta_Seconds();
			Pawn pawn = Possessed;
			Bouncer_Zombie zombie = <Bouncer_Zombie>pawn;

			Vec3 offset = zombie.Collision_Center - zombie.Position;

			Move_Towards(zombie.Position + (m_vec_to_jump_target), BASE_JUMP_SPEED);

			float elapsed_delta = (m_jump_elapsed / m_jump_duration);
			zombie.Update_Jump_Delta(elapsed_delta);

			m_jump_elapsed += delta;
			if (m_jump_elapsed >= m_jump_duration)
			{
				change_state(Idle);
			}
	
			Track_Pawn();
			Store_Last_State();
		}
	}   

	bool Move_Towards(Vec3 target, float speed)
	{ 
		Pawn pawn = Possessed;
		
		Vec3 start_position = pawn.Position;
		Vec3 final_position = start_position;
		float delta = Time.Get_Delta();
		bool at_target = false;

		// Apply movement.
		Vec3 vector_to_target = (target - final_position).Unit();
		Vec3 movement = ((vector_to_target * speed) * delta);

	//	m_movement_delta = Math.Lerp(m_movement_delta, movement, 0.2f);
	//	movement = m_movement_delta;

		final_position = final_position + movement;
		Vec3 vector_to_target_after = (target - final_position).Unit();

		// TODO: Should probably prevent movement overshooting target here.

		// If this movement will take us onto or past the target, then we are at the end of our movement.
		at_target = vector_to_target.Dot(vector_to_target_after) < 0.0 || vector_to_target_after == Vec3(0.0f);

		// Turn towards direction of movement.
		Vec3 position_delta = final_position - pawn.Position;
		// Pervent flickering if we move one direction then the other in quick succession
		m_movement_delta = Math.Lerp(m_movement_delta, position_delta, 0.2f);
		Direction dir = Direction_Helper.Calculate_Direction(m_movement_delta.X, m_movement_delta.Y, m_old_direction);
		pawn.Turn_Towards(dir);

		// Onwards!
		pawn.Move_Towards(final_position);

		return at_target;
	}	 

	// -------------------------------------------------------------------------
	// General functions.
	// -------------------------------------------------------------------------
	override void On_Possession_Changed(Pawn other)
	{	
        base.On_Possession_Changed(other);

		if (other != null)
		{
            Bouncer_Zombie bouncer = <Bouncer_Zombie>other;
            if (bouncer)
            {
                bouncer.Reset_Jump_State();
            }
		}
	}	
    
}
