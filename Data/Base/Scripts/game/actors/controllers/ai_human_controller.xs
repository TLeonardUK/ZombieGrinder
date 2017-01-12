 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using game.actors.player.human_player;
using game.actors.controllers.player_human_controller;
using system.time;
using game.weapons.pistol.weapon_pistol;
using game.actors.decoration.movable.movable;
using game.skills.skill_effect_manager;

public enum AI_Human_State
{
	Find_Initial_Breadcrumb,
	Moving_To_Breadcrumb,
	Interact_At_Breadcrumb,
	Wait_At_Breadcrumb,

    Incapacitated,

	Moving_To_Pickup,
}

[
	Name("AI Human Controller"), 
	Description("Basic AI to navigate through maps.") 
]
public class AI_Human_Controller : Player_Human_Controller
{ 
	private AI_Breadcrumb m_breadcrumb = null;
	private AI_Human_State m_human_state = AI_Human_State.Find_Initial_Breadcrumb;
	private float m_wait_time_left = 0.0f;
	private Pickup m_pickup_target = null;
	private Vec3 m_aim_vector;
	private bool m_aiming_at_enemy = false;
	private bool m_fire_down_last_frame = false;
	private bool m_deseperate_for_ammo = false;
	private bool m_at_risk = false;
	private Vec3 m_enemy_average_position;
	private float m_at_risk_time = 0.0f;
	private float m_at_risk_cooldown = 0.0f;
	private Pickup[] m_ignored_pickups = new Pickup[0];
	private float m_pickup_time = 0.0f;

	// -------------------------------------------------------------------------
	//  Move to the next breadcrumb!
	// -------------------------------------------------------------------------
	void Next_Breadcrumb()
	{
		if (m_breadcrumb.Link != "")
		{
			Actor[] next = Scene.Find_Actors_By_Tag(m_breadcrumb.Link);	
			m_breadcrumb = null;

			if (next.Length() > 0)
			{
				AI_Breadcrumb next_breadcrumb = <AI_Breadcrumb>next[0];
				if (next_breadcrumb != null)
				{
					m_breadcrumb = next_breadcrumb;
					m_human_state = AI_Human_State.Moving_To_Breadcrumb;
					return;
				}
			}
		}

		m_breadcrumb = null;
		m_human_state = AI_Human_State.Find_Initial_Breadcrumb;
	}
	
	// -------------------------------------------------------------------------
	//  Updates movement.
	// -------------------------------------------------------------------------
	void Update_AI_Movement()
	{
		Vec3 movement = Vec3(0.0f);
		bool bCanDivertForPickups = true;
        Human_Player human_pawn = <Human_Player>this.Possessed;
		
		// Work out movement.
		switch (m_human_state)
		{	

		// --------------------------------------------------------------------------------------------------------------
		//	Main breadcrumb following states.
		// --------------------------------------------------------------------------------------------------------------
		case AI_Human_State.Find_Initial_Breadcrumb:
			{
				if (m_breadcrumb == null)
				{
					m_breadcrumb = <AI_Breadcrumb>Scene.Find_Closest_Actor(typeof(AI_Breadcrumb), this.Possessed.Collision_Center);
				}
				if (m_breadcrumb != null)
				{	
					m_human_state = AI_Human_State.Moving_To_Breadcrumb;
				}
				break;
			}
		case AI_Human_State.Moving_To_Breadcrumb:
			{
				path_planner.Source_Position = this.Possessed.Collision_Center;
				path_planner.Target_Position = m_breadcrumb.Center;
				movement = path_planner.Movement_Vector.Unit() * 10.0f;

				if ((path_planner.Source_Position - path_planner.Target_Position).Length() < 8.0f)
				{
					if (m_breadcrumb.bInteractAtEnd)
					{
						m_human_state = AI_Human_State.Interact_At_Breadcrumb;
						m_wait_time_left = 2.0f;
					} 
					else if (m_breadcrumb.bWaitFor > 0.0f)
					{
						m_human_state = AI_Human_State.Wait_At_Breadcrumb;
						m_wait_time_left = m_breadcrumb.bWaitFor;
					}
					else
					{	
						Next_Breadcrumb();
					}
				}
				
				break;
			}
		case AI_Human_State.Interact_At_Breadcrumb:
			{
				m_wait_time_left -= Time.Get_Delta_Seconds();
				m_interact_down = true;
				bCanDivertForPickups = false;
				if (m_wait_time_left < 0.0f)
				{
					Next_Breadcrumb();
				}
				break;
			}
		case AI_Human_State.Wait_At_Breadcrumb:
			{
				m_wait_time_left -= Time.Get_Delta_Seconds();
				if (m_wait_time_left < 0.0f)
				{
					Next_Breadcrumb();
				}
				break;
			}

		// --------------------------------------------------------------------------------------------------------------
		//	Incapacitated
		// --------------------------------------------------------------------------------------------------------------
		case AI_Human_State.Incapacitated:
			{      
                bCanDivertForPickups = false;      
                           
                Actor closest_human = Scene.Find_Closest_Actor(typeof(Human_Player), this.Possessed.Collision_Center, this.Possessed);
                if (closest_human)
                {
				    path_planner.Source_Position = this.Possessed.Collision_Center;
				    path_planner.Target_Position = closest_human.Center;
			    	movement = path_planner.Movement_Vector.Unit() * 10.0f;
                }

                if (!human_pawn.Is_Incapacitated())
                { 
				    m_human_state = AI_Human_State.Find_Initial_Breadcrumb;                    
                }

				break;
			}

		// --------------------------------------------------------------------------------------------------------------
		//	Side-objectives, pickups etc.
		// --------------------------------------------------------------------------------------------------------------
		case AI_Human_State.Moving_To_Pickup:
			{
				bCanDivertForPickups = false;

				m_pickup_time += Time.Get_Delta_Seconds();

				if (m_pickup_target == null ||
					!Scene.Is_Spawned(m_pickup_target) ||
					m_pickup_target.Is_Spawned == false ||
					m_pickup_target.Purchasable == true ||
					!(<Interactable>m_pickup_target).Can_Interact(this.Possessed) ||
					m_pickup_target.AI_Attractive == false)
				{
					Log.Write("Pickup no longer intresting, back to breadcrumbs!");
					m_human_state = AI_Human_State.Find_Initial_Breadcrumb;
					m_pickup_target = null;
				}
				else
				{
					if ((path_planner.Source_Position - path_planner.Target_Position).Length() < 8.0f)
					{
						Log.Write("Got to pickup, back to breadcrumbs!");
						m_ignored_pickups.AddLast(m_pickup_target);
						m_human_state = AI_Human_State.Find_Initial_Breadcrumb;
						m_pickup_target = null;
						break;
					}
				}
				
				if (m_pickup_target != null)
				{
					path_planner.Source_Position = this.Possessed.Collision_Center;
					path_planner.Target_Position = m_pickup_target.Center;
					movement = path_planner.Movement_Vector.Unit() * 10.0f;

					if (!path_planner.Has_Path && m_pickup_time > 5.0f)
					{
						m_ignored_pickups.AddLast(m_pickup_target);
						m_human_state = AI_Human_State.Find_Initial_Breadcrumb;
						m_pickup_target = null;
						Log.Write("Could not generate path to pickup after "+m_pickup_time+" seconds, adding to ignore list.");
						break;
					}
				}
				break;
			}
		}

        // Incapacitated?
        if (m_human_state != AI_Human_State.Incapacitated && human_pawn.Is_Incapacitated())
        { 
			m_human_state = AI_Human_State.Incapacitated;                    
        }
		 
		// Divert for pickups?
		if (bCanDivertForPickups == true)
		{
			float max_distance = 256.0f;
			if (m_deseperate_for_ammo)
			{
				max_distance = 1024.0f;
			}

			Actor[] actors = Scene.Find_Actors_In_Radius(typeof(Pickup), max_distance, this.Possessed.Collision_Center);
			foreach (Pickup pickup in actors)
			{
				if (!m_ignored_pickups.Contains(pickup))
				{
					if (pickup.AI_Attractive == true && pickup.Is_Spawned == true && pickup.Purchasable == false && (<Interactable>pickup).Can_Interact(this.Possessed))
					{
						Log.Write("Found pickup diversion, moving towards.");
						m_pickup_target = pickup;
						m_human_state = AI_Human_State.Moving_To_Pickup;
						m_pickup_time = 0.0f;
						break;
					}
				}
			}

            // Occluded by locked door, find key
	    	actors = Scene.Find_Actors_In_Radius(typeof(Dungeon_Door), 64.0f, this.Possessed.Collision_Center);

            bool bNeedKey = false;
            foreach (Dungeon_Door door in actors)
            {
                if (door.Is_Locked)
                { 
                    bNeedKey = true;
                    break;
                }
            }

            if (bNeedKey)
            { 
                bool bHasKey = (Inventory_Pickup.Get_Inventory_Item_For_Net_ID(this.Possessed.Net_ID) as Key_Pickup) != null;
                if (bHasKey)
                { 
				    Log.Write("Attempting to interact with locked door.");
                    m_interact_down = true;
                }
                else
                { 
			        Actor[] key_pickups = Scene.Find_Actors(typeof(Key_Pickup));
                    Key_Pickup best_key = null;
                    float best_distance = 0.0f;
                    
                    foreach (Key_Pickup pickup in key_pickups)
                    {
                        if (!pickup.Is_Spawned)
                        { 
                            continue;
                        }

                        float distance = (pickup.Collision_Center - this.Possessed.Collision_Center).Length();
                        if (best_key == null || distance < best_distance)
                        { 
                            best_key = pickup;
                            best_distance = distance;
                        }
                    }

                    if (best_key)
                    {
					    Log.Write("Close to locked door, looking for and moving to key!.");
					    m_pickup_target = <Pickup>best_key;
					    m_human_state = AI_Human_State.Moving_To_Pickup;
					    m_pickup_time = 0.0f;
                    }
                }
            }
		}
        
        // If incapacitated human is near us, hold interact.        
		Actor[] near_players = Scene.Find_Actors_In_Radius(typeof(Human_Player), 64.0f, this.Possessed.Collision_Center);
        foreach (Human_Player player in near_players)
        {
            if (player != this.Possessed)
            { 
                if (player.Is_Incapacitated())
                { 
                    m_interact_down = true;          
                    if (!m_at_risk)
                    {          
					    movement = Vec3(0.0f);
                    }
                }
            }
        }

        // If we are at risk then try and move away from enemies until they are dead, ignore breadcrumbs for now.
        if (m_at_risk == true && !(m_deseperate_for_ammo && m_human_state == AI_Human_State.Moving_To_Pickup)) 
		{
			movement = (this.Possessed.Collision_Center - (m_enemy_average_position - this.Possessed.Collision_Center)).Unit() * 10.0f;
		}

		// Set movement keys based off movement.
		m_move_vector = Math.Lerp(m_move_vector, movement.Unit(), 0.1f);
	}
	
	// -------------------------------------------------------------------------
	//  Updates character weapon aiming.
	// -------------------------------------------------------------------------
	void Update_AI_Aiming()
	{		
		Vec3 aiming = Vec3(0.0f);
		
		// Aiming direction defaults to movement direction.
		aiming = m_move_vector;
		m_aiming_at_enemy = false;

		// If enemy close, aim at them.
		Actor[] enemies = Scene.Find_Actors_In_Radius(typeof(Pawn), 140.0f, this.Possessed.Center);
		Vec3 total_pos = Vec3(0.0f);

		bool enemies_in_outer_radius = false;
		bool enemies_in_inner_radius = false;

		m_at_risk_cooldown -= Time.Get_Delta_Seconds();

		foreach (Pawn enemy in enemies)
		{
			Vec3 dist = (enemy.Center - this.Possessed.Center);

			if (enemy.Team_Index == this.Possessed.Team_Index)
			{
				continue;
			}

			float risk_factor = 0.0;
			Enemy real_enemy = <Enemy>enemy;
			if (real_enemy != null)
			{
				risk_factor = real_enemy.AI_RiskFactor;
			}

			if (dist.Length() < 140.0f)
			{
				aiming = dist;
				m_aiming_at_enemy = true;
			}
			if (dist.Length() < 80.0f * risk_factor && risk_factor > 0.0f)
			{			
				enemies_in_outer_radius = true;
			}
			if (dist.Length() < 50.0f * risk_factor && risk_factor > 0.0f)
			{			
				enemies_in_inner_radius = true;
			}

			total_pos = total_pos + enemy.Center;
		}

		if (m_at_risk == true)
		{
			if (m_at_risk_cooldown > 0.0f || !enemies_in_outer_radius || enemies.Length() <= 0)
			{
				m_at_risk = false;
				m_at_risk_time = 0.0f;
			}
		}
		else
		{
			if (enemies_in_inner_radius && Cheat_Manager.bGodMode == false)
			{
				m_at_risk = true;
			}
		}

		if (m_at_risk)
		{
			m_enemy_average_position = total_pos / <float>enemies.Length();
			m_at_risk_time += Time.Get_Delta_Seconds();
			if (m_at_risk_time > 30.0f)
			{
				m_at_risk_cooldown = 20.0f;
				m_at_risk_time = 0.0f;
			}
		}

		// Aim in direction!
		m_aim_vector = Math.Lerp(m_aim_vector, aiming, 0.1f);
		m_direction = Direction_Helper.Calculate_Direction(m_aim_vector.X, m_aim_vector.Y, m_direction, false);
		this.Possessed.Turn_Towards(m_direction);
	}
	
	// -------------------------------------------------------------------------
	//  Updates character weapon firing.
	// -------------------------------------------------------------------------
	void Update_AI_Weapon()
	{
		// Choose weapon to switch to.
		Weapon best_weapon = null;
		float best_weapon_priority = 0.0f;
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{			
			Weapon weapon = this.Possessed.Get_Weapon_Slot(i);
			if (weapon != null)
			{
				float priority = weapon.AI_Priority;
				if (!weapon.Has_Ammo())
				{
					priority = 0.0f;
				}

				if (priority > best_weapon_priority)
				{
					best_weapon_priority = priority;
					best_weapon = weapon;
				}
			}
		}

		// Switch weapon.
		if (best_weapon != null && best_weapon != this.Possessed.Get_Active_Weapon())
		{
			this.Possessed.Set_Active_Weapon_Slot(best_weapon.Slot);
		}

		// If no weapons have ammo, flag us as desperate for ammo so we can find guns.
		m_deseperate_for_ammo = (best_weapon == null);

		// If we are aiming at an enemy, fire.
		if (m_aiming_at_enemy)
		{
			Weapon active_weapon = this.Possessed.Get_Active_Weapon();
			Projectile_Weapon proj_weapon = <Projectile_Weapon>active_weapon;
			bool is_automatic = false;

		//	if (proj_weapon != null)
		//	{
		//		is_automatic = proj_weapon.Is_Automatic;
		//	}

			if (is_automatic)
			{
				m_fire_down = true;
			}
			else
			{				
				m_fire_down = !m_fire_down_last_frame;
			}
		}
	}

	// -------------------------------------------------------------------------
	//  Calculate what input to plug into the controller.
	// -------------------------------------------------------------------------
	override void Update_Input()
	{		
		if (this.Possessed == null)
		{
			return;
		}

		// Reset keys.		
		m_interact_down = false;
		m_move_by_vector = true;
		m_fire_down_last_frame = m_fire_down;
		m_fire_down = false;

		// Update movement.
		Update_AI_Movement();

		// Update aiming.
		Update_AI_Aiming();

		// Update weapon switching/firing.
		Update_AI_Weapon();
	}		

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{
		PathPlannerComponent path_planner
		{
			Regenerate_Delta = 64.0f;
			Collision_Group  = CollisionGroup.Player;
			Client_Side		 = true;
		}
	}
}