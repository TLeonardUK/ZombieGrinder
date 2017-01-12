 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.actors.pawn;
using game.actors.player.human_player;
using system.time;
using game.weapons.pistol.weapon_pistol;
using game.actors.decoration.movable.movable;
using game.skills.skill_effect_manager;

[
	Name("Player Human Controller"), 
	Description("Allows for the player to control a human character.") 
]
public class Player_Human_Controller : Pawn_Controller
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
	protected bool		m_skillslot1_down				= false;
	protected bool		m_skillslot2_down				= false;
	protected bool		m_skillslot3_down				= false;
	protected bool		m_drop_coins_down				= false;
	protected bool		m_drop_weapon_down				= false;
    protected float     m_drop_weapon_down_time         = 0.0f;
    protected float     m_drop_coins_down_time          = 0.0f;
    protected bool      m_can_drop_weapon               = false;

	protected bool		m_prev_interact_down			= false;
	protected bool		m_prev_strafe_down				= false;
	protected float		m_auto_fire_interval            = 0.0f;

	protected int		m_old_profile_change_count		= 0;

	protected float		m_ghost_acceleration_x			= 0.0f;
	protected float		m_ghost_acceleration_y			= 0.0f;
	
	protected Interactable	m_interact_target			= null;
	protected float			m_interact_time				= 0.0f;
	protected float			m_interact_duration			= 0.0f;
	protected string		m_interact_prompt			= "";
	protected float			m_interact_prompt_persist	= 0.0f;

	protected bool			m_move_by_vector			= false;
	protected Vec3			m_move_vector;

	protected bool			m_sprinting					= false;
	protected float			m_sprint_timer				= 0.0f;
	protected bool			m_sprint_up_down			= false;
	protected bool			m_sprint_down_down			= false;
	protected bool			m_sprint_left_down			= false;
	protected bool			m_sprint_right_down			= false;
	protected bool			m_spawned_sprint_effect		= false;

	public int				m_interact_other_id			= -1;
	public float			m_interact_duration_delta	= 0.0f;
    
    private Actor           m_intro_target              = null;
    private bool            m_in_intro                  = false;
    private int             m_intro_frames              = 0;
    private float           m_intro_finish_timer        = 0.0f;
    private float           m_intro_start_timer         = 0.0f;

    const float             INTRO_START_DELAY           = 0.4f;
    const float             INTRO_FINISH_DELAY          = 0.4f;

    private Player_Spawn    m_queued_intro              = null;
    private bool            m_outro_finished            = false;

	// -------------------------------------------------------------------------
	// Base constants.
	// -------------------------------------------------------------------------
	const float BASE_WALK_SPEED         = 16.0;	
    const float INTRO_WALK_SPEED        = 11.0f;

	const float GHOST_ACCELERATION = 5.0f;
	const float GHOST_MAX_ACCELERATION = 10.0f;
	const float GHOST_DECELERATION_LERP = 0.4f;

	const float MAX_INTERACTION_DISTANCE = 36.0f;
	
	const float	SPRINT_SPEED_MULTIPLIER		= 5.5f;
	const float	SPRINT_SPEED_DURATION		= 0.75f;
	const float SPRINT_STAMINA_COST			= 60.0f;

	// -------------------------------------------------------------------------
	// This state just waits for a pawn to control.
	// -------------------------------------------------------------------------
	default state Waiting_For_Possession
	{
	}
	 
	// -------------------------------------------------------------------------
	// Basic walking state.
	// -------------------------------------------------------------------------
	state Walking
	{
		event void On_Enter()
		{
			Store_Last_State();
		}		
		event void On_Tick()
		{
			NetUser player = Owner;

			if (Network.Is_Server())
			{
				Update_Items();
			}

			if (player.Is_Local)
			{
				Update_Input();
				Update_Sprint();
				Update_Movement();
				
				Pawn pawn = Possessed;
				Human_Player human = <Human_Player>pawn;

				if (human != null && !human.Is_Incapacitated())
				{
					Update_Inventory();
					Update_Shooting();
				}
				
				if (human != null && !human.Is_Incapacitated())
				{
					Update_Interaction();
				}
				else
				{
					m_interact_target = null;
					m_interact_time = 0.0f;
					m_interact_prompt = "";
				}

				Store_Last_State();
			}	
			
			// Track pawn position.
			if (this.Possessed != null)
			{
				this.Position = this.Possessed.Position;
			}
            
            if (m_queued_intro)
            {
                Start_Intro(m_queued_intro);
                m_queued_intro = null;
            }
			
			Update_Sprint_Effect();
		}
	}   
	
	// -------------------------------------------------------------------------
	//  Gets and stores current input.
	// -------------------------------------------------------------------------
	virtual void Update_Input()
	{
		Pawn pawn = Possessed;
		Human_Player human = <Human_Player>pawn;
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
		m_skillslot1_down				= Input.Was_Pressed(player, OutputBindings.SkillSlot1);
		m_skillslot2_down				= Input.Was_Pressed(player, OutputBindings.SkillSlot2);
		m_skillslot3_down				= Input.Was_Pressed(player, OutputBindings.SkillSlot3);

		m_drop_coins_down				= Input.Is_Down(player, OutputBindings.DropCoins);
		m_drop_weapon_down				= Input.Is_Down(player, OutputBindings.DropWeapon);
    
    
        // Deal with firing while aiming if the user wants it.
        if (m_look_up_down || m_look_down_down || m_look_left_down || m_look_right_down)
        {
			if (Options.Get_Bool("fire_when_aiming_enabled") && pawn)
            {
		        /*Weapon active = pawn.Get_Active_Weapon();
                if (active.Is_Automatic)
                {*/
                    m_fire_down = true;
                /*}
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
            m_skillslot1_down               = false;
            m_skillslot2_down               = false;
            m_skillslot3_down               = false;
            m_drop_coins_down               = false;
            m_drop_weapon_down              = false;
        }

        if (human && human.Reversed_Controls)
        {
            bool tmp = m_up_down;
            m_up_down = m_down_down;
            m_down_down = tmp;

            tmp = m_left_down;
            m_left_down = m_right_down;
            m_right_down = tmp;
        }

		if (Minimap.Is_Open())
		{
			m_up_down = false;
			m_down_down = false;
			m_left_down = false;
			m_right_down = false;
		}
	}	
	
	// -------------------------------------------------------------------------
	//  Updates interaction between player and other pawns.
	// -------------------------------------------------------------------------
	string Make_Interact_Prompt(Interactable i, Pawn other)
	{
		Pawn pawn = Possessed;
		float price_multiplier = 1.0f;
		
		if (pawn != null)
		{
			price_multiplier = pawn.Get_Price_Multiplier() * this.Owner.Stat_Multipliers[Profile_Stat.Buy_Price];
		}

		string prompt = i.Get_Prompt(other);
		int cost = i.Get_Cost(other) * price_multiplier;

		if (prompt == "")
		{
			return "";
		}

		if (cost != 0)
		{
			prompt = prompt + Locale.Get("#prompt_price_postfix").Format(new object[] { <object>cost });
		}

		return prompt;
	}

    rpc void Spend_Money(int cost)
    { 
        this.Owner.Coins = Math.Max(0, this.Owner.Coins - cost);
    }                       

	void Update_Interaction()
	{
        Pawn pawn = Possessed;
		float price_multiplier = 1.0f;
		
		if (pawn != null)
		{
			price_multiplier = pawn.Get_Price_Multiplier() * this.Owner.Stat_Multipliers[Profile_Stat.Buy_Price];
		}

		if (m_interact_target != null)
		{
			if (Scene.Is_Spawned(<object>m_interact_target) &&
				m_interact_target.Can_Interact(Possessed) &&
                !pawn.Is_Incapacitated())
			{
				m_interact_other_id			= (<Actor>(<object>m_interact_target)).Net_ID;
				m_interact_duration_delta	= m_interact_time / m_interact_duration;
			}
		}
		else
		{
			m_interact_other_id = -1;
		}

		if (pawn.Is_Interact_Locked())
		{	
			m_interact_target = null;
			m_interact_time = 0.0f;
			m_interact_prompt = "";
		
			return;
		}

		if (m_interact_down == true)
		{
			if (m_interact_target != null)
			{
				if (!Scene.Is_Spawned(<object>m_interact_target) || 
					!m_interact_target.Can_Interact(Possessed))
				{
					m_interact_target = null;
					m_interact_time = 0.0f;

					//Log.Write("Stopped interaction due to target despawning or no longer able to interact.");
				}
				else
				{
					m_interact_time += Time.Get_Frame_Time();
					if (m_interact_time >= m_interact_duration)
					{
						//Log.Write("Finished interaction.");
						
						int cost = (m_interact_target.Get_Cost(Possessed) * price_multiplier);
                        rpc(RPCTarget.All, RPCTarget.None) Spend_Money(cost);						

						m_interact_target.Interact(Possessed);

						m_interact_target = null;
						m_interact_time = 0.0f;
						m_interact_prompt = "";
					}
				}
			}
			else if (m_prev_interact_down == false)
			{
				Actor[] potential = Scene.Find_Actors_In_Radius(
					typeof(Interactable), 
					MAX_INTERACTION_DISTANCE,
					this.Possessed.Collision_Center
				);
		
				foreach (Interactable i in potential)
				{
					if (i.Can_Interact(Possessed))
					{
						//Log.Write("Starting interaction.");

						int balance_delta = 0;

						if (Owner != null)
						{
							balance_delta = this.Owner.Coins - <int>(i.Get_Cost(Possessed) * price_multiplier);
						}
						
						if (balance_delta < 0)
						{
							m_interact_prompt		  = Locale.Get("#not_enough_coins_prompt").Format(new object[] { <object>Math.Abs(balance_delta) });
							m_interact_prompt_persist = 4.0f;
							m_interact_target		  = null;

							Audio.Play2D("sfx_hud_ui_foodvendingmachine_error");
						}
						else
						{
							m_interact_prompt	= Make_Interact_Prompt(i, Possessed);
							m_interact_duration = i.Get_Duration(Possessed);
							m_interact_target	= i;
							m_interact_time		= 0.0f;
						}

						break;
					}
				}
			}
		}
		else if (m_interact_target != null)
		{
			m_interact_target = null;
			m_interact_time = 0.0f;
			m_interact_prompt = "";
		
			//Log.Write("Finished interaction prematurely.");
		}
		else 
		{
			if (m_interact_prompt_persist <= 0.0)
			{
				Actor[] potential = Scene.Find_Actors_In_Radius(
					typeof(Interactable), 
					MAX_INTERACTION_DISTANCE,
					this.Possessed.Collision_Center
				);
		
				m_interact_prompt = "";

				foreach (Interactable i in potential)
				{
					if (i.Can_Interact(Possessed))
					{
						m_interact_prompt = Make_Interact_Prompt(i, Possessed);
						break;
					}
				}
			}
			else
			{
				m_interact_prompt_persist -= Time.Get_Delta();
			}
		}
	}

	public override bool Is_Interacting()
	{
		return (m_interact_target != null);
	}
	
	public override float Get_Interact_Progress()
	{
		return (m_interact_target == null) ? 0.0f : m_interact_time / m_interact_duration;
	}

	public override string Get_Interact_Prompt()
	{
		if (Movable.Is_Pawn_Moving(Possessed))
		{
			return Locale.Get("#place_prompt");
		}
		else
		{
			return m_interact_prompt;
		}
	}

	// -------------------------------------------------------------------------
	//  Inventory management.
	// -------------------------------------------------------------------------
	void Update_Inventory()
	{
		Pawn pawn = Possessed;
	
        // Drain durability of items.        
		NetUser user = this.Owner;
        if (user)
        {
            for (int i = 0; i < Item_Slot.COUNT; i++)
            {
                Item weapon_item = user.Local_Profile.Get_Item_Slot(<Item_Slot> i);
                if (weapon_item != null)
                {
                    weapon_item.Equip_Duration += Time.Get_Delta_Seconds();
                }
            }
        }

		if (pawn.Is_Interact_Locked())
		{
			return;
		}

        bool bWeaponCycled = false;

		// Weapon hot-keys.
		if (m_weapon_slot_1_down && pawn.Get_Weapon_Slot(0) != null)
		{
			pawn.Set_Active_Weapon_Slot(0);
            bWeaponCycled = true;
		}
		if (m_weapon_slot_2_down && pawn.Get_Weapon_Slot(1) != null)
		{
			pawn.Set_Active_Weapon_Slot(1);
            bWeaponCycled = true;
		} 
		if (m_weapon_slot_3_down && pawn.Get_Weapon_Slot(2) != null)
		{
			pawn.Set_Active_Weapon_Slot(2);
            bWeaponCycled = true;
		}
		if (m_weapon_slot_4_down && pawn.Get_Weapon_Slot(3) != null)
		{
			pawn.Set_Active_Weapon_Slot(3);
            bWeaponCycled = true;
		}
		if (m_weapon_slot_5_down && pawn.Get_Weapon_Slot(4) != null)
		{
			pawn.Set_Active_Weapon_Slot(4);
            bWeaponCycled = true;
		}
		if (m_weapon_slot_6_down && pawn.Get_Weapon_Slot(5) != null)
		{
			pawn.Set_Active_Weapon_Slot(5);
            bWeaponCycled = true;
		}
		
		// Gamepad cycle weapon keys.
		if (m_weapon_cycle_backward_down)
		{
			pawn.Cycle_Weapon_Backward();
            bWeaponCycled = true;
		}
		if (m_weapon_cycle_foreward_down)
		{
			pawn.Cycle_Weapon_Forward();
            bWeaponCycled = true;
		}

		// Activate skills.
		if (m_skillslot1_down == true)
		{
			Activate_Skill(0);
		}
		else if (m_skillslot2_down == true)
		{
			Activate_Skill(1);
		}
		else if (m_skillslot3_down == true)
		{
			Activate_Skill(2);
		}

        // Drop items.
        if (m_drop_coins_down)
        {
            m_drop_coins_down_time += Time.Get_Delta_Seconds();
        }
        else
        {
            if (m_drop_coins_down_time > 0.0f && m_drop_coins_down_time < 0.2f)
            {
                Drop_Coins();            
            }
            m_drop_coins_down_time = 0.0f;
        }

        if (m_drop_weapon_down)
        {
            m_drop_weapon_down_time += Time.Get_Delta_Seconds();
            if (m_drop_weapon_down_time > 0.5f && m_can_drop_weapon)
            {
                Drop_Weapon();
                m_can_drop_weapon = false;
            }
        }
        else
        {
            m_drop_weapon_down_time = 0.0f;
            m_can_drop_weapon = true;
        }

        if (bWeaponCycled)
        {
			// Notify others of the alt-fire (tutorials hook this)
			Event_Pawn_Cycle_Weapon reloaded = new Event_Pawn_Cycle_Weapon();
			reloaded.Instigator = pawn;
			reloaded.Fire_Global();
        }
	}

    void Drop_Weapon()
    {
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;

		Pawn pawn = Possessed;
		Weapon weapon = pawn.Get_Weapon_Slot(pawn.Get_Active_Weapon_Slot());

        if (weapon != null && 
            weapon.GetType() != typeof(Weapon_Pistol) && 
            mode.Allow_Manual_Weapon_Drops)
        {
            rpc(RPCTarget.Server, RPCTarget.None) RPC_Drop_Weapon(pawn.Get_Active_Weapon_Slot(), pawn.Get_Direction());
        }
    }

    rpc void RPC_Drop_Weapon(int index, int direction)
    {
		Pawn pawn = Possessed;    
		Weapon weapon = pawn.Get_Weapon_Slot(index);

        if (weapon == null)
        {
            return;
        }

        if (weapon.Pickup_Sprite == "")
        {
            return;
        }

        pawn.Remove_Weapon_Slot(index);

        Log.Write("Player " + this.Net_ID + " has dropped weapon index " + index);    

        if (index == Weapon_Slot.Pistol)
        {
		    pawn.Give_Weapon(typeof(Weapon_Pistol));
        }

		Manual_Weapon_Drop_Pickup drop = <Manual_Weapon_Drop_Pickup>Scene.Spawn(typeof(Manual_Weapon_Drop_Pickup), null);
        drop.Weapon_Type            = weapon.GetType();
        drop.Ammo_Type              = (weapon.Ammo_Modifier != null ? weapon.Ammo_Modifier.GetType() : null);
        drop.Pickup_Sprite          = weapon.Pickup_Sprite;
        drop.Pickup_Sound           = weapon.Pickup_Sound;
        drop.Pickup_Name            = weapon.Pickup_Name;
        drop.Clip_Size              = weapon.Clip_Size;
        drop.Reserve_Size           = weapon.Reserve_Size;
        drop.Upgrade_Tree_Ids       = weapon.Upgrade_Ids;
        drop.Upgrade_Item_Archetype = weapon.Upgrade_Item_Archetype;
        drop.Reserve_Ammo           = weapon.Reserve_Ammo;
        drop.Clip_Ammo              = weapon.Clip_Ammo;
        drop.Primary_Color          = weapon.Primary_Color;
		drop.Layer                  = pawn.Layer;

        drop.In_Intro               = true;
        drop.Intro_Timer            = 0.0f;
        drop.Intro_Length           = 0.3f;
        drop.Start_Height           = 16.0f;
        drop.End_Height             = 3.0f;
        drop.Start_Position         = (pawn.Collision_Center - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f));
        drop.End_Position           = (pawn.Collision_Center - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f)) + (Direction_Helper.Vectors[direction] * 24.0f);
        drop.Position               = drop.Start_Position;

        drop.Lifetime               = 30.0f;

        drop.Update_Sprite();

        rpc(RPCTarget.All, RPCTarget.None) RPC_Play_Drop_Sound();
    }

    void Drop_Coins()
    {    
		Pawn pawn = Possessed;

        int quantity = Math.Min(500, this.Owner.Coins);
        if (quantity > 0)
        {
            this.Owner.Coins -= quantity;
            rpc(RPCTarget.Server, RPCTarget.None) RPC_Drop_Coins(quantity, this.Owner.Coins, pawn.Get_Direction());
        }
    }

    rpc void RPC_Drop_Coins(int coins, int result_coins, int direction)
    {
		Pawn pawn = Possessed;

        Log.Write("Player " + this.Net_ID + " has dropped " + coins);

        if (!this.Owner.Is_Local)
        {
            this.Owner.Coins = result_coins;
        }

		Coin_Bag drop = <Coin_Bag>Scene.Spawn(typeof(Coin_Bag), null);
        drop.Coin_Quantity = coins;
		drop.Layer = pawn.Layer;

        drop.In_Intro               = true;
        drop.Intro_Timer            = 0.0f;
        drop.Intro_Length           = 0.3f;
        drop.Start_Height           = 16.0f;
        drop.End_Height             = 3.0f;
        drop.Start_Position         = (pawn.Collision_Center - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f));
        drop.End_Position           = (pawn.Collision_Center - Vec3(drop.Bounding_Box.Width * 0.5f, drop.Bounding_Box.Height * 0.5f, 0.0f)) + (Direction_Helper.Vectors[direction] * 24.0f);
        drop.Position               = drop.Start_Position;
        drop.Lifetime               = 30.0f;

        drop.Update_Sprite();

        rpc(RPCTarget.All, RPCTarget.None) RPC_Play_Drop_Sound();
    }

    rpc void RPC_Play_Drop_Sound()
    {
		Pawn pawn = Possessed;
		Audio.Play3D("sfx_objects_pickups_item_drop", pawn.Center);
    }

	// -------------------------------------------------------------------------
	//  Skill management.
	// -------------------------------------------------------------------------
	void Activate_Skill(int index)
	{		
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		bool is_pvp = mode != null && mode.Is_PVP();

		Pawn pawn = Possessed;
		Human_Player human = <Human_Player>pawn;
		
		if (human.Skills_Disabled)
		{
			return;
		}
		
		if (is_pvp)
		{
			return;
		}			

		NetUser user = this.Owner;

		Skill skill = user.Profile.Get_Skill_Slot(index);
		if (skill != null)
		{			
			Skill_Archetype archetype = Skill_Archetype.Find_By_ID(skill.Archetype_ID);

			bool is_active = Skill_Effect_Manager.Is_Active(archetype, this.Owner);
			if (!is_active)
			{            
				bool has_energy = Cheat_Manager.Infinite_Energy;		

                if (!has_energy)                
                {
                    if (user.Local_Profile.Get_Skill_Energy(archetype) >= archetype.Energy_Required)
                    {
                        has_energy = true;   
                    }
                }

				if (has_energy)
				{
					// Remove gem that powers this.
					user.Local_Profile.Set_Skill_Energy(archetype, 0.0f);
					
					// Ask server to activate!
					rpc(RPCTarget.All, RPCTarget.None) RPC_Skill_Activated(skill.Archetype_ID);
				}
				else
				{
					Audio.Play2D("sfx_hud_score_decrease");
					Log.Write("Attempt to activate skill failed, not enough energy.");
				}
			}
			else
			{				
				Audio.Play2D("sfx_hud_score_decrease");
				Log.Write("Attempt to activate skill failed, already active.");
			}
		}
	}
	
	private rpc void RPC_Skill_Activated(int archetype_id) 
	{
		Skill_Archetype archetype = Skill_Archetype.Find_By_ID(archetype_id);
		Skill_Effect_Manager.Activate(archetype, this.Owner);
	}

	// -------------------------------------------------------------------------
	//  Item management.
	// -------------------------------------------------------------------------
	void Update_Items()
	{
		// Update inventory if profile has changed.
		int profile_change_count = this.Owner.Profile_Change_Counter;
		if (profile_change_count != m_old_profile_change_count)
		{
			Setup_Items();
		}
	}

	// -------------------------------------------------------------------------
	//  Basic shooting code.
	// -------------------------------------------------------------------------
	void Update_Shooting()
	{
		Pawn pawn = Possessed;
		
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
				// Notify others of the alt-fire (tutorials hook this)
				Event_Pawn_Alt_Fire reloaded = new Event_Pawn_Alt_Fire();
				reloaded.Instigator = pawn;
				reloaded.Fire_Global();

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

			// Notify others of the manual reload (tutorials hook this)
			Event_Pawn_Manual_Reload reloaded = new Event_Pawn_Manual_Reload();
			reloaded.Reloader = pawn;
			reloaded.Fire_Global();
		}

		// Out of pisol-weapon ammo, give the user a normal pistol.		
		Weapon weapon = pawn.Get_Weapon_Slot(Weapon_Slot.Pistol);
		if (weapon != null && weapon.GetType() != typeof(Weapon_Pistol))
		{
			if (!weapon.Has_Ammo())
			{			
				pawn.Give_Weapon(typeof(Weapon_Pistol));
			}
		}
	}
	
	// -------------------------------------------------------------------------
	//  Basic movement code.
	// -------------------------------------------------------------------------
	float Get_Speed()
	{
		Pawn pawn = Possessed;
		float multiplier = 1.0f;

		if (pawn != null)
		{
			multiplier *= pawn.Get_Speed_Multiplier();
		}		

		if (m_sprinting == true)
		{
			float sprint_factor = Math.BezierCurveLerp(1.0f, SPRINT_SPEED_MULTIPLIER, 1.0f, 0.75f, m_sprint_timer);
			multiplier *= sprint_factor;
		}

		return BASE_WALK_SPEED * multiplier;
	}      

	void Update_Sprint()
	{
		Pawn pawn = Possessed;
		Human_Player human = <Human_Player>pawn;
	
		if (human != null && human.Is_Incapacitated())
		{
			m_sprinting = false;
			return;
		}

		if (m_sprinting)
		{
			m_left_down = m_sprint_left_down;
			m_right_down = m_sprint_right_down;
			m_up_down = m_sprint_up_down;
			m_down_down = m_sprint_down_down;

			m_sprint_timer += Time.Get_Delta_Seconds() / SPRINT_SPEED_DURATION;
			if (m_sprint_timer >= 1.0f)
			{
				Log.Write("Sprint finish.");
				m_sprinting = false;
			}
		}
		else
		{
			if (m_sprint_down == true && pawn.Stamina >= SPRINT_STAMINA_COST)
			{
				// Notify others of the sprint (tutorials hook this)
				Event_Pawn_Sprint reloaded = new Event_Pawn_Sprint();
				reloaded.Instigator = pawn;
				reloaded.Fire_Global();

				m_sprint_timer = 0.0f;
				m_sprinting = true;

				pawn.Stamina -= SPRINT_STAMINA_COST;

				m_sprint_left_down = m_left_down;
				m_sprint_right_down = m_right_down;
				m_sprint_up_down = m_up_down;
				m_sprint_down_down = m_down_down;
			
				if (!m_left_down && !m_right_down && !m_up_down && !m_down_down)
				{
					switch (pawn.Get_Direction())
					{
					case Direction.S:
						{
							m_sprint_down_down = true;
							break;
						}
					case Direction.SE:
						{
							m_sprint_down_down = true;
							m_sprint_right_down = true;
							break;
						}
					case Direction.E:
						{
							m_sprint_right_down = true;
							break;
						}
					case Direction.NE:
						{
							m_sprint_right_down = true;
							m_sprint_up_down = true;
							break;
						}
					case Direction.N:
						{
							m_sprint_up_down = true;
							break;
						}
					case Direction.NW:
						{
							m_sprint_up_down = true;
							m_sprint_left_down = true;
							break;
						}
					case Direction.W:
						{
							m_sprint_left_down = true;
							break;
						}
					case Direction.SW:
						{
							m_sprint_down_down = true;
							m_sprint_left_down = true;
							break;
						}
					}
				}
			}
		}
	}

	void Update_Sprint_Effect()
	{
		Pawn pawn = Possessed;
		
		if (m_sprinting)
		{
			if (m_spawned_sprint_effect == false)
			{
				Effect eff = FX.Spawn("human_sprint", pawn.Position, 0, pawn.Layer, pawn);
				eff.DisposeOnFinish = true;
				eff.Attach_To_Offset(pawn, Vec3(0.0f, 24.0f, 0.0f));

				Audio.Play3D("sfx_objects_avatars_sprint", pawn.Center);

				m_spawned_sprint_effect = true;
			}
		}
		else
		{
			m_spawned_sprint_effect = false;
		}
	}
	
	void Update_Movement()
	{ 
		Pawn pawn = Possessed;
		Human_Player human = <Human_Player>pawn;
		Weapon current_weapon = pawn.Get_Active_Weapon();
		
		Vec3 final_position = pawn.Position;
		float delta = Time.Get_Delta();
		float speed = Get_Speed();

		// Hack hack hack hakc.
		if (human != null)
		{
			human.m_movement_anim_multiplier = (speed / BASE_WALK_SPEED) * 0.75f;
		}

		bool move_pressed = (m_left_down || m_right_down || m_up_down || m_down_down);
		bool look_pressed = (m_look_left_down || m_look_right_down || m_look_up_down || m_look_down_down);
		
		// If we are incapped we do a bit of slow-slidy-acceleration effect for our speed. 
		if (human != null && human.Is_Incapacitated())
		{
			if (m_move_by_vector)
			{
				m_ghost_acceleration_x = m_ghost_acceleration_x + (m_move_vector.X * GHOST_ACCELERATION * delta);
				m_ghost_acceleration_y = m_ghost_acceleration_y + (m_move_vector.Y * GHOST_ACCELERATION * delta);

				if (m_move_vector.X > -0.01f && m_move_vector.X < -0.01f)
				{
					m_ghost_acceleration_x = Math.Lerp(m_ghost_acceleration_x, 0.0f, GHOST_DECELERATION_LERP * delta);
				}
				if (m_move_vector.Y > -0.01f && m_move_vector.Y < -0.01f)
				{
					m_ghost_acceleration_y = Math.Lerp(m_ghost_acceleration_y, 0.0f, GHOST_DECELERATION_LERP * delta);
				}
			}
			else
			{
				if (m_left_down)	
				{
					m_ghost_acceleration_x -= GHOST_ACCELERATION * delta;
				}
				if (m_right_down)	
				{
					m_ghost_acceleration_x += GHOST_ACCELERATION * delta;
				}
				if (m_up_down)		
				{
					m_ghost_acceleration_y -= GHOST_ACCELERATION * delta;
				}
				if (m_down_down)
				{
					m_ghost_acceleration_y += GHOST_ACCELERATION * delta;
				}

				if (!m_left_down && !m_right_down)
				{
					m_ghost_acceleration_x = Math.Lerp(m_ghost_acceleration_x, 0.0f, GHOST_DECELERATION_LERP * delta);
				}
				if (!m_up_down && !m_down_down)
				{
					m_ghost_acceleration_y = Math.Lerp(m_ghost_acceleration_y, 0.0f, GHOST_DECELERATION_LERP * delta);
				}
			}

			m_ghost_acceleration_x = Math.Clamp(m_ghost_acceleration_x, -GHOST_MAX_ACCELERATION, GHOST_MAX_ACCELERATION);
			m_ghost_acceleration_y = Math.Clamp(m_ghost_acceleration_y, -GHOST_MAX_ACCELERATION, GHOST_MAX_ACCELERATION);

			final_position.X += m_ghost_acceleration_x * delta;
			final_position.Y += m_ghost_acceleration_y * delta;
		}
		else
		{
			// Move based on input.
			if (m_move_by_vector)
			{
				final_position = final_position + (m_move_vector * speed * delta);
			}
			else
			{
				// If sprinting diagonally, move at half speed. We should to this for normal movement
				// as well, but it feels better without it.
				if ((m_up_down && (m_left_down || m_right_down)) ||
					(m_down_down && (m_left_down || m_right_down)))
				{
					if (m_sprinting)
					{
						speed *= 0.75f;
					}
				}

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
			}

			m_ghost_acceleration_x = 0.0f;
			m_ghost_acceleration_y = 0.0f;
		}

		// Strafing?
		if (m_strafe_down && !m_prev_strafe_down)
		{			
			// Notify others of the strafe (tutorials hook this)
			Event_Pawn_Strafe reloaded = new Event_Pawn_Strafe();
			reloaded.Instigator = pawn;
			reloaded.Fire_Global();
		}
		
		// Direction for local players is calculated based on intended movement, not collsion-basec movement (otherwise you auto-face
		// other directions when you hit obstacles).			
		if (m_strafe_down == false && (move_pressed || look_pressed) && (current_weapon == null || !current_weapon.Is_Direction_Locked()) && !m_sprinting)
		{
			// Turn based on twin-stick look.
			if (look_pressed)
			{
				Direction dir = Direction_Helper.Calculate_Direction(<int>m_look_right_down - <int>m_look_left_down, <int>m_look_down_down - <int>m_look_up_down, m_old_direction);
				pawn.Turn_Towards(dir);
			}

			// Turn based on movement.
			else
			{
				Vec3 position_delta = final_position - pawn.Position;
				Direction dir = Direction_Helper.Calculate_Direction(position_delta.X, position_delta.Y, m_old_direction);
				pawn.Turn_Towards(dir);
			}
		}
  
		// Onwards!
		pawn.Move_Towards(final_position);
	}
	
	void Store_Last_State()
	{	
		m_old_direction = m_direction;
	}
	
	// -------------------------------------------------------------------------
	// Intro/Outro cutscene animations.
	// -------------------------------------------------------------------------	
    void Queue_Intro(Player_Spawn target)
	{
        m_queued_intro = target;
	}
	void Start_Intro(Player_Spawn target)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Start_Intro(target.Unique_ID);
	}
	void Start_Outro(Player_Spawn_Cutscene_Point target)
	{    
        m_outro_finished = false;    
		rpc(RPCTarget.All, RPCTarget.None) RPC_Start_Outro(target.Unique_ID);
	}
	
	rpc void RPC_Start_Intro(int target_id)
	{
		Player_Spawn target = <Player_Spawn>Scene.Get_Unique_ID_Actor(target_id);
		if (target != null)
		{
			Log.Write("Starting intro for player.");

            m_intro_target = target;
            change_state(Intro);
		}
		else
		{
			Log.Write("Failed to start player intro, could not find target with id " + target_id + ".");
		}
	}
	rpc void RPC_Start_Outro(int target_id)
	{
		Player_Spawn_Cutscene_Point target = <Player_Spawn_Cutscene_Point>Scene.Get_Unique_ID_Actor(target_id);
		if (target != null)
		{
			Log.Write("Starting outro for player.");
            
            m_intro_target = target;
            change_state(Intro);
		}
		else
		{
			Log.Write("Failed to start player outro, could not find target with id " + target_id + ".");
		}
	}

    void Finish_Intro()
    {
		rpc(RPCTarget.All, RPCTarget.None) RPC_Finish_Intro();
    }

    rpc void RPC_Finish_Intro()
    {
        m_outro_finished = true;
        change_state(Walking);
    }
	
	bool Is_In_Outro()
	{
		return m_in_intro;
	}

    bool Is_Outro_Finished()
    {
        return m_outro_finished;
    }

	state Intro
	{
		event void On_Enter()
		{
			Store_Last_State();
            m_in_intro = true;
            m_intro_frames = 0;
            m_intro_finish_timer = 0.0f;
            m_intro_start_timer = 0.0f;

		    Human_Player pawn = <Human_Player>Possessed;
            pawn.Begin_Intro();            
		}		
		event void On_Exit()
		{
			Store_Last_State();
            m_in_intro = false;
            
		    Human_Player pawn = <Human_Player>Possessed;
            pawn.End_Intro();
		}		
		event void On_Tick()
		{
			NetUser player = Owner;
		    Pawn pawn = Possessed;
		
			if (player.Is_Local)
			{
                m_intro_start_timer += Time.Get_Delta_Seconds();
                if (m_intro_start_timer > INTRO_START_DELAY)
                {
				    // Plan a path and move towards the target.
				    intro_path_planner.Source_Position = pawn.Collision_Center;
				    intro_path_planner.Target_Position = m_intro_target.Collision_Center;

                    Vec3 chase_vector = intro_path_planner.Movement_Vector;
			
				    // Move towards the chase vector.
                    if ((intro_path_planner.At_Target && m_intro_frames > 0) || m_intro_finish_timer > 0.0f)
                    {
                        Player_Spawn spawn = <Player_Spawn>m_intro_target;
                        if (spawn)
                        {
                            pawn.Turn_Towards(spawn.Spawn_Direction);
                        }

                        m_intro_finish_timer += Time.Get_Delta_Seconds();
                        if (m_intro_finish_timer > INTRO_FINISH_DELAY)
                        {
                            Log.Write("Reached end of outro path.");
                            Finish_Intro();
                        }
                    }
                    else
                    {                  
                        Move_Towards(pawn.Position + chase_vector, INTRO_WALK_SPEED, true, m_old_direction);
                    }

                    m_intro_frames++;
				}

				Store_Last_State();
			}	
			
			// Track pawn position.
			if (this.Possessed != null)
			{
				this.Position = this.Possessed.Position;
			}

			Update_Sprint_Effect();
		}
	}   
		
	// -------------------------------------------------------------------------
	// General functions.
	// -------------------------------------------------------------------------
	override void On_Possession_Changed(Pawn other)
	{	
		if (other != null)
		{
			if (Network.Is_Server())
			{	
				Setup_Weapons();
				Setup_Items();
			}
			
            Log.Write("Moving to walking state for player controller "+this.Net_ID);
			change_state(Walking);
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

	void Setup_Weapons()
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		mode.Setup_Player_Weapons(this.Possessed, this.Owner.Profile);
	}
	
	void Setup_Items()
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;
		mode.Setup_Player_Items(this.Possessed, this.Owner.Profile);

		m_old_profile_change_count = this.Owner.Profile_Change_Counter;
	}

	replicate(ReplicationPriority.Low)
	{	
		m_sprinting
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		m_interact_other_id
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
		}
		m_interact_duration_delta
		{
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
        m_in_intro
        {
			Owner = ReplicationOwner.Client;
			Mode  = ReplicationMode.Absolute;
        }
	}

	// -------------------------------------------------------------------------
	//  Components.
	// -------------------------------------------------------------------------
	components
	{
		PathPlannerComponent intro_path_planner
		{
			Regenerate_Delta = 64.0f;
			Collision_Group  = CollisionGroup.Environment; // We don't want player as if we are using this for intros we want to pass through player-only obstacles.
			Client_Side		 = true;
		}
	}
}
