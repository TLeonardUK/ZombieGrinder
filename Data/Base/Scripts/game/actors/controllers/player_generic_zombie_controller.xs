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
	Name("Player Generic Zombie Controller"), 
	Description("Base class for player zombie controllers that use generic controls.") 
]
public class Player_Generic_Zombie_Controller : Controller
{
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	protected Direction 	m_old_direction					= Direction.S;
	protected Direction 	m_direction						= Direction.S;
    
	protected bool		m_up_down						= false;
	protected bool		m_down_down						= false;
	protected bool		m_left_down						= false;
	protected bool		m_right_down					= false;
	protected bool		m_look_up_down					= false;
	protected bool		m_look_down_down				= false;
	protected bool		m_look_left_down				= false;
	protected bool		m_look_right_down				= false;
	protected bool		m_fire_down						= false;
	protected bool		m_alt_fire_down					= false;
	protected bool		m_weapon_slot_1_down 			= false;
	protected bool		m_weapon_slot_2_down 			= false;
	protected bool		m_weapon_slot_3_down 			= false;
	protected bool		m_weapon_slot_4_down 			= false;
	protected bool		m_weapon_slot_5_down 			= false;
	protected bool		m_weapon_slot_6_down 			= false;
	protected bool		m_weapon_cycle_backward_down 	= false;
	protected bool		m_weapon_cycle_foreward_down 	= false;		
	protected bool		m_strafe_down					= false;	
	protected bool		m_sprint_down					= false;
	protected bool		m_reload_down					= false;
	protected bool		m_interact_down					= false;
	protected bool		m_prev_fire_down				= false;
    protected bool      m_fire_down_real                = false;

	protected bool		m_prev_interact_down			= false;
	protected bool		m_prev_strafe_down				= false;

	protected float		m_auto_fire_interval            = 0.0f;

    protected bool      bLockDirectionTo4               = false;
    protected bool      bLockDirectionTo2               = false;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float GENERIC_WALK_SPEED = 12.0f;
	
	Player_Generic_Zombie_Controller()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
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
		}		
		event void On_Tick()
		{
			NetUser player = Owner;
	    	Pawn pawn = Possessed;
            
			if (player.Is_Local)
			{
				Update_Input();
				Update_Movement();
				Update_Shooting();
            }

            if (pawn)
            {
                pawn.Damage_Multiplier = 0.5f;
            }

            Idle_Tick();

            Track_Pawn();
			Store_Last_State();
		}
	}   
	
    virtual void Idle_Tick()
    {
    }

	// -------------------------------------------------------------------------
	//  Gets and stores current input.
	// -------------------------------------------------------------------------
	virtual void Update_Input()
	{
		Pawn pawn = Possessed;
		NetUser player = Owner;

		m_prev_interact_down = m_interact_down;
		m_prev_strafe_down = m_strafe_down;
			
		m_up_down 						= Input.Is_Down(player, OutputBindings.Up);
		m_down_down 					= Input.Is_Down(player, OutputBindings.Down);
		m_left_down 					= Input.Is_Down(player, OutputBindings.Left);
		m_right_down 					= Input.Is_Down(player, OutputBindings.Right);
		m_look_up_down 					= Input.Is_Down(player, OutputBindings.LookUp);
		m_look_down_down 				= Input.Is_Down(player, OutputBindings.LookDown);
		m_look_left_down 				= Input.Is_Down(player, OutputBindings.LookLeft);
		m_look_right_down 				= Input.Is_Down(player, OutputBindings.LookRight);
		m_fire_down						= Input.Is_Down(player, OutputBindings.Fire);
		m_alt_fire_down					= Input.Is_Down(player, OutputBindings.AltFire);
		m_strafe_down 					= Input.Is_Down(player, OutputBindings.Strafe);
		m_sprint_down 					= Input.Was_Pressed(player, OutputBindings.Sprint);
		m_interact_down 				= Input.Is_Down(player, OutputBindings.Interact);
		m_reload_down 					= Input.Was_Pressed(player, OutputBindings.Reload);
		m_weapon_slot_1_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot1);
		m_weapon_slot_2_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot2);
		m_weapon_slot_3_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot3);
		m_weapon_slot_4_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot4);
		m_weapon_slot_5_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot5);
		m_weapon_slot_6_down 			= Input.Was_Pressed(player, OutputBindings.WeaponSlot6);
		m_weapon_cycle_backward_down	= Input.Was_Pressed(player, OutputBindings.WeaponCycleBackward);
		m_weapon_cycle_foreward_down 	= Input.Was_Pressed(player, OutputBindings.WeaponCycleForward);
          
        // Deal with firing while aiming if the user wants it.
        if (m_look_up_down || m_look_down_down || m_look_left_down || m_look_right_down)
        {
			if (Options.Get_Bool("fire_when_aiming_enabled") && pawn)
            {
		       /* Weapon active = pawn.Get_Active_Weapon();
                if (active.Is_Automatic)
                {*/
                    m_fire_down = true;
               /* }
                else
                {
                    if (m_auto_fire_interval <= 0.0f)
                    {
                        m_fire_down = true;
                        m_auto_fire_interval = active.Fire_Interval * 0.25f;
                    }
                }

                m_auto_fire_interval -= Time.Get_Delta_Seconds();*/
            }
        }

		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
        if (mode.In_Cutscene_Mode())
        {
		    m_up_down 						= false;
		    m_down_down 					= false;
		    m_left_down 					= false;
		    m_right_down 					= false;
		    m_look_up_down 					= false;
		    m_look_down_down 				= false;
		    m_look_left_down 				= false;
		    m_look_right_down 				= false;
		    m_fire_down						= false;
		    m_alt_fire_down					= false;
		    m_strafe_down 					= false;
		    m_sprint_down 					= false;
		    m_interact_down 				= false;
		    m_reload_down 					= false;
		    m_weapon_slot_1_down 			= false;
		    m_weapon_slot_2_down 			= false;
		    m_weapon_slot_3_down 			= false;
		    m_weapon_slot_4_down 			= false;
		    m_weapon_slot_5_down 			= false;
		    m_weapon_slot_6_down 			= false;
		    m_weapon_cycle_backward_down	= false;
		    m_weapon_cycle_foreward_down 	= false;

        }
	}	
	
	// -------------------------------------------------------------------------
	//  Basic shooting code.
	// -------------------------------------------------------------------------
    virtual bool Can_Shoot()
    {
        return true;
    }

	void Update_Shooting()
	{
		Pawn pawn = Possessed;
        
        // Don't allow decapitated 
		if (Can_Shoot() == false)
        {
            return;
        }
		
		// Update normal firing.
		if (m_fire_down && !pawn.m_ignore_firing_until_no_input)
		{
			if (!pawn.Is_Firing())
			{
				pawn.Begin_Fire();
			}
		} 
		else  
		{
			if (pawn.Is_Firing())
			{	
				pawn.Finish_Fire();
			}    
		}
		
		// Update alt firing.
		if (m_alt_fire_down && !pawn.m_ignore_firing_until_no_input)
		{
			if (!pawn.Is_Alt_Firing())
			{						
				pawn.Begin_Alt_Fire();				
			}
		}
		else
		{
			if (pawn.Is_Alt_Firing())
			{	
				pawn.Finish_Alt_Fire();				
			}
		}	

		// Re-enable firing?
		if (!m_fire_down && !m_alt_fire_down)
		{
			pawn.m_ignore_firing_until_no_input = false;
		}
		
		// Reload weapon.
		if (m_reload_down)
		{
			pawn.Begin_Reload();
		}
	}

	// -------------------------------------------------------------------------
	//  Basic movement code.
	// -------------------------------------------------------------------------    
    virtual bool Can_Move()
    {
        return true;
    }

	void Update_Movement()
	{ 
		Pawn pawn = Possessed;
		Enemy zombie = <Enemy>pawn;
		Weapon current_weapon = pawn.Get_Active_Weapon();
		
        if (!Can_Move())
        {
            return;
        }

        // Cannot move while being shocked or frozen.
        if (zombie.m_status_ailment == StatusAilmentType.Shocked ||
            zombie.m_status_ailment == StatusAilmentType.Frozen)
        {
            return;
        }

		Vec3 final_position = pawn.Position;
		float delta = Time.Get_Delta();
		float speed = Get_Speed();

		bool move_pressed = (m_left_down || m_right_down || m_up_down || m_down_down);
		bool look_pressed = (m_look_left_down || m_look_right_down || m_look_up_down || m_look_down_down);
		
		if (m_left_down)	
		{
			final_position.X -= speed * delta;
		}
		if (m_right_down)	
		{
			final_position.X += speed * delta;
		}
		if (m_up_down)		
		{
			final_position.Y -= speed * delta;
		}
		if (m_down_down)
		{
			final_position.Y += speed * delta;
		}
		
		// Direction for local players is calculated based on intended movement, not collsion-basec movement (otherwise you auto-face
		// other directions when you hit obstacles).			
		if (m_strafe_down == false && (move_pressed || look_pressed) && (current_weapon == null || !current_weapon.Is_Direction_Locked()))
		{
			// Turn based on twin-stick look.
			if (look_pressed)
			{
                if (bLockDirectionTo2)
                {
                    if (m_look_down_down)
                    {
    				    pawn.Turn_Towards(Direction.S);
                    }
                    else if (m_look_up_down)
                    {
    				    pawn.Turn_Towards(Direction.N);
                    }
                }
                else
                {
				    Direction dir = Direction_Helper.Calculate_Direction(<int>m_look_right_down - <int>m_look_left_down, <int>m_look_down_down - <int>m_look_up_down, m_old_direction, bLockDirectionTo4);
				    pawn.Turn_Towards(dir);
                }
			}

			// Turn based on movement.
			else
			{
				Vec3 position_delta = final_position - pawn.Position;
                if (bLockDirectionTo2)
                {
                    if (position_delta.Y > 0.0f)
                    {
    				    pawn.Turn_Towards(Direction.S);
                    }
                    else if (position_delta.Y < 0.0f)
                    {
    				    pawn.Turn_Towards(Direction.N);
                    }
                }
                else
                {
				    Direction dir = Direction_Helper.Calculate_Direction(position_delta.X, position_delta.Y, m_old_direction, bLockDirectionTo4);
				    pawn.Turn_Towards(dir);
                }
			}
		}
  
		// Onwards!
		pawn.Move_Towards(final_position);
	}

	virtual float Get_Speed()
	{
		float speed_modifier = this.Possessed.Get_Ailment_Speed_Multiplier();
		return GENERIC_WALK_SPEED * Difficulty_Manager.Enemy_Speed_Multiplier * speed_modifier;
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
	}
}
