// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.components.collision;
using system.time; 
using system.actors.effect;
using system.fx;

// Different types of status ailments a pawn can have.
public enum StatusAilmentType
{
	None,
	Burnt,
	Frozen,
	Shocked
}

[
	Name("Pawn"), 
	Description("Base class for actors which can be controlled by controllers.") 
]
public class Pawn : Actor
{
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------	
	public serialized(1) Weapon[]		m_weapon_slots			= new Weapon[Weapon_Slot.COUNT];      
	public serialized(1) int			m_active_weapon_slot 	= 0;
	public serialized(1) Item[]			m_item_slots			= new Item[Item_Slot.COUNT];    

	public serialized(1) int			Team_Index				= 0; 
	public serialized(1) float			Health					= 100.0f;
	public serialized(1) float			Max_Health				= 100.0f;
	public serialized(1) float			Armor					= 0.0f;
	public serialized(1) float			Max_Armor				= 100.0f;
	public				 float			Stamina					= 100.0f;
	public				 float			Max_Stamina				= 100.0f;
	public serialized(1) Direction		Current_Direction		= Direction.S;

	public serialized(1) bool			Is_Dead					= false;
	
	// Used by HUD to render lerped health and armour values. Bit
	// nicer than jumps in health bar.
	public float Display_Health = 100.0f;
	public float Display_Armor  = 100.0f;
	public float Display_Stamina = 100.0f;
    
    public float Hud_Element_Alpha = 1.0f;

	protected bool m_firing	  = false;
	protected bool m_alt_firing = false;

	private const float DEATH_DEFER_INTERVAL = 1500f;
	private const float DEACTIVATE_COMPONENT_INTERVAL = 150.0f;
	private float m_death_start = 0.0f;
	   
	public Controller Possessor = null;

	public float m_accumulated_damage = 0.0f;
	public float m_accumulated_damage_time = 0.0f;
	
	private const float DAMAGE_THROTTLE_THRESHOLD = 6.0f;
	private const float DAMAGE_THROTTLE_TIME_THRESHOLD = 0.2f;
	
	private const float STATUS_AILMENT_PROBABILITY = 0.3f;
	private const float STATUS_AILMENT_DURATION = 6.0f;
    private const float STATUS_AILMENT_COOLOFF = 1.75f;

	public StatusAilmentType m_status_ailment;
	protected float m_status_ailment_time_left;
	protected Effect m_status_ailment_effect;
	protected bool m_status_ailment_spawned_initial_effect;
	protected Actor m_status_ailment_instigator;
    protected float m_status_ailment_cooloff = 0.0f;
	
	// Some general public information about how we were harmed last - This will not be in sync all the time
	// between server and client. Just use it for rough-local stuff like achievements.
	public int Last_Damage_Penetration_Count; 
	public int Last_Damage_Ricochet_Count; 
	public CollisionDamageType Last_Damage_Type; 

	public bool m_ignore_firing_until_no_input = false;

    // Pawn specific damage multiplier.
    public float Damage_Multiplier = 1.0f;

	// -------------------------------------------------------------------------
	//  Sets up the newly spawned pawn.
	// -------------------------------------------------------------------------
	public void Setup()
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			if (m_weapon_slots[i] != null)
			{
				m_weapon_slots[i].Owner = this;
			}
		}
	}
	
	// Gets the team this actor belongs on.
	public override int Get_Team_Index()
	{
		return Team_Index;
	}
	
	// -------------------------------------------------------------------------
	// Status ailments.
	// -------------------------------------------------------------------------
	private rpc void RPC_Set_Status_Ailment(int instigator_id, StatusAilmentType ailment, float duration, int item_type_id)
	{
		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);

		Type item_type_type = Type.Find(item_type_id);
		Item_Archetype item_type = null;
		if (item_type_type != null)
		{
			item_type = Item_Archetype.Find_By_Type(item_type_type);
		}
		
		Log.Write("Pawn " + this.Net_ID + " has been given status ailment "+ailment+" for "+duration+".");
		
		Event_Pawn_Status_Ailment evt = new Event_Pawn_Status_Ailment();
		evt.Effected		= this;
		evt.Last_Ailment	= m_status_ailment;
		evt.Ailment			= ailment;
		evt.Instigator		= instigator;
		evt.Fire_Global();
		
		// Ailment duration modifier.
		if (instigator != null)
		{
			Human_Player player = <Human_Player>instigator;
			if (player != null)
			{
				duration *= player.Owner.Stat_Multipliers[Profile_Stat.Enemy_Status_Duration];
			}
			
			if (item_type && instigator.Owner && instigator.Owner.Is_Local && m_status_ailment == StatusAilmentType.None)
			{
				instigator.Owner.Local_Profile.Increment_Item_Stat(item_type, Profile_Item_Stat_Type.Ailments_Inflicted, 1);					
			}
		}

		m_status_ailment = ailment;
		m_status_ailment_time_left = duration;
		m_status_ailment_spawned_initial_effect = false;
		m_status_ailment_instigator = instigator;
	}
	
	virtual void Set_Status_Ailment(Actor instigator, StatusAilmentType ailment, float duration, Item_Archetype weapon_type)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Set_Status_Ailment(instigator != null ? instigator.Net_ID : -1, ailment, duration, weapon_type == null ? -1 : weapon_type.GetType().ID);
	}
    
	virtual void Clear_Ailment()
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Clear_Ailment();
	}

	rpc void RPC_Clear_Ailment()
	{
		m_status_ailment = StatusAilmentType.None;
		m_status_ailment_time_left = 0.0f;
	}

	bool IsParalyzed()
	{
		return (m_status_ailment == StatusAilmentType.Frozen || m_status_ailment == StatusAilmentType.Shocked);
	}

	virtual void Tick_Ailment()
	{
		float delta_t = Time.Get_Delta_Seconds();

		// Status ailment finished?
		m_status_ailment_time_left -= delta_t;
        m_status_ailment_cooloff -= delta_t;
		if (m_status_ailment_time_left <= 0.0f)
		{
            if (m_status_ailment != StatusAilmentType.None)
            {
         	   m_status_ailment = StatusAilmentType.None;
               m_status_ailment_cooloff = STATUS_AILMENT_COOLOFF;
            }
		}

		string ailment_effect = "";
		Vec3   ailment_effect_pos = Position;		

		switch (m_status_ailment)
		{
		case StatusAilmentType.Frozen:
			{
				// Spawn the frozen particle.
				if (m_status_ailment_spawned_initial_effect == false)
				{
					ailment_effect = "pawn_frozen";
				}
				
				Vec4 world_bb = this.World_Bounding_Box;
				ailment_effect_pos = Vec3(world_bb.X + (world_bb.Width * 0.5f), world_bb.Y + world_bb.Height, 0.0f);

				break;
			}
		case StatusAilmentType.Burnt:
			{
				// Spawn fire effects over body.
				ailment_effect = "pawn_burnt";
				ailment_effect_pos = this.Center;
				ailment_effect_pos.Y += (this.Bounding_Box.Height * 0.25f);
				break;
			}
		case StatusAilmentType.Shocked:
			{
				// Spawn lightning effects over our body.
				ailment_effect = "pawn_shocked";
				ailment_effect_pos = this.Center;
				ailment_effect_pos.Y += (this.Bounding_Box.Height * 0.25f);
				break;
			}
		}

		// Get rid of despawned instigator?
		if (m_status_ailment_instigator != null && !Scene.Is_Spawned(m_status_ailment_instigator))
		{
			m_status_ailment_instigator = null;
		}

		// Finish effect?
		if (m_status_ailment_effect != null)
		{
			if (!Scene.Is_Spawned(m_status_ailment_effect))
			{
				m_status_ailment_effect = null;
			}
			else
			{					
				m_status_ailment_effect.Position = ailment_effect_pos;
			}
		}
		else if (ailment_effect != "")
		{
			m_status_ailment_effect = FX.Spawn(ailment_effect, ailment_effect_pos, 0.0f, this.Layer, m_status_ailment_instigator);
			m_status_ailment_effect.DisposeOnFinish = true;						
			m_status_ailment_spawned_initial_effect = true;
		}
	}

	public float Get_Ailment_Speed_Multiplier()
	{
		if (m_status_ailment == StatusAilmentType.Frozen ||
			m_status_ailment == StatusAilmentType.Shocked)
		{
			return 0.0f;
		}
		return 1.0f;
	}

	public Vec4 Get_Ailment_Color()
	{
		if (m_status_ailment == StatusAilmentType.Burnt)
		{
			return Vec4(100.0f, 100.0f, 100.0f, 255.0f);
		}
		return Vec4(255.0f, 255.0f, 255.0f, 255.0f);
	}

	// -------------------------------------------------------------------------
	//  Health/Damage management.
	// -------------------------------------------------------------------------
	public void Setup_Health(float health)
	{
		Health = health;
		Max_Health = health;

	//	Log.Write("Pawn has max health of "+health);
	}

	public virtual bool Can_Target()
	{
		return true;
	}

	public bool Is_Friendly_Fire(Actor instigator)
	{
		if (instigator == null)
		{
			return false;
		}

		// If instigator is ourselves, it dosen't count as FF. We need to deal with this seperately for self-damage.
		if (instigator == this)
		{
			return false;
		}

		// Deal with imposing-actors (turrets etc).
		if (instigator.Get_Team_Index() == Get_Team_Index())
		{
			return true;
		}
	
		return false;
	}
	
	public float Calculate_Damage_Multiplier(Actor instigator, float damage, Weapon_SubType weapon_sub_type)
	{
		// Our own damage-taken multiplier.
		if (this.Owner != null)
		{				
			damage *= this.Owner.Stat_Multipliers[Profile_Stat.Damage_Taken];
		}

		if (instigator != null)
		{
			Pawn instigator_pawn = <Pawn>instigator;
			if (instigator_pawn != null)
			{
				damage *= instigator_pawn.Get_Damage_Multiplier();
			}

			// Weapon-type specific multipliers.
			if (instigator.Owner != null)
			{
				damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Damage];

				switch (weapon_sub_type)
				{
					case Weapon_SubType.Melee:
					{
						damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Melee_Damage];
						break;
					}
					case Weapon_SubType.Heavy:
					{
						damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Heavy_Weapon_Damage];
						break;
					}
					case Weapon_SubType.Small_Arms:
					{
						damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Small_Arms_Damage];
						break;
					}
					case Weapon_SubType.Rifle:
					{
						damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Rifle_Damage];
						break;
					}
					case Weapon_SubType.Healing:
					{
						damage *= instigator.Owner.Stat_Multipliers[Profile_Stat.Healing_Speed];
						break;
					}
				}
			}            
		}

        // Pawn specific damage multiplier.
        damage *= Damage_Multiplier;			
		
		return damage;
	}

	public bool Apply_Damage(
		CollisionComponent component, 
		Actor instigator, 
		Vec3 position, 
		float dir, 
		float damage, 
		CollisionDamageType type, 
		int ricochet_count, 
		int penetration_count,
		bool bApplyDamage,
		Item_Archetype item_type)
	{
		if (instigator != null && Is_Friendly_Fire(instigator) && damage >= 0.0f)
		{
			if (Network.Is_Server())
			{
				//Log.Write("Pawn " + Net_ID + " recieved " + damage + " friendly fire, ignoring.");
				
			}
			return false;
		}
		else
		{
			Last_Damage_Ricochet_Count = ricochet_count;
			Last_Damage_Penetration_Count = penetration_count;
			Last_Damage_Type = type;
			
			// Track damage stats.
			if (bApplyDamage && item_type && instigator != null && instigator.Owner != null && instigator.Owner.Is_Local)
			{
                float original = Health;
				Health = Math.Max(0.0f, Math.Min(Max_Health, Health - damage));
                float diff = original - Health;
				instigator.Owner.Local_Profile.Increment_Item_Stat(item_type, Profile_Item_Stat_Type.Damage_Inflicted, diff);				
			}

			// Apply damage!
			if (bApplyDamage && (Network.Is_Server() || (this.Owner != null && this.Owner.Is_Local)))
			{
				if (damage > 0.0f)
				{
					float armor_amount = Armor - Math.Max(0.0f, Armor - damage);
					Armor = Math.Max(0.0f, Armor - armor_amount);
					Health = Math.Max(0.0f, Health - (damage - armor_amount));
				}
				else if (damage < 0.0f)
				{
					Health = Math.Min(Max_Health, Health - damage);
				}

				if (Health > 0.0f && Network.Is_Server())// && Math.Rand(0.0f, 1.0f) < STATUS_AILMENT_PROBABILITY)
				{
					// Apply status ailment.
					StatusAilmentType ailment = StatusAilmentType.None;
					switch (type)
					{
					case CollisionDamageType.Fire:
						{
							ailment = StatusAilmentType.Burnt;
							break;
						}
					case CollisionDamageType.Ice:
						{
							ailment = StatusAilmentType.Frozen;
							break;
						}
					case CollisionDamageType.Shock:
						{
							ailment = StatusAilmentType.Shocked;
							break;
						}
					}

					if (ailment != StatusAilmentType.None && ailment != m_status_ailment)
					{
                        if (m_status_ailment_cooloff <= 0.0f)
                        {
                            Set_Status_Ailment(instigator, ailment, STATUS_AILMENT_DURATION, item_type);
                        }
                        else
                        {
                        //    m_status_ailment_cooloff = STATUS_AILMENT_COOLOFF;
                        }
					}
				}
			}
			return true;
		}

		return false;
	}

	public virtual void Heal(float health, float armor)
	{
		Armor  = Math.Clamp(Armor + armor, 0.0f, Max_Armor);
		Health = Math.Clamp(Health + health, 0.0f, Max_Health);
	}

	public bool Throttle_Damage_FX(float damage)
	{
		m_accumulated_damage += damage;

		bool show_fx = false;

		if (Health <= 0.0f ||
			Math.Abs(m_accumulated_damage) >= DAMAGE_THROTTLE_THRESHOLD ||
			m_accumulated_damage_time <= 0.0f)
		{
			show_fx = true;
            m_accumulated_damage_time = DAMAGE_THROTTLE_TIME_THRESHOLD;
		}

		return show_fx;
	}

	public void Tick_Throttle_Damage()
	{
		m_accumulated_damage_time -= Time.Get_Delta_Seconds();
	}
	
	public float Get_Throttled_Damage()
	{
		float tmp = m_accumulated_damage;
		m_accumulated_damage = 0.0f;
		return Math.Ceiling(tmp);
	}
	
	public override Direction Get_Direction()
	{
		return Current_Direction;
	}

	public int Get_Shots_Fired_Delta()
	{
		Weapon active = Get_Active_Weapon();
		if (active != null)
		{
			return active.Get_Shots_Fired_Delta();
		}
	}

	public void Despawn()
	{ 
		// Despawn this object.
		Scene.Despawn(this);

		// Tell controller we are no more.
		if (Possessor != null)
		{
			Possessor.On_Possessed_Despawned();
		}
	}

	private rpc void RPC_Die(int instigator_id, int weapon_type_id)
	{
		// Don't allow duplicate deaths.
		if (Is_Dead)
		{
			return;
		}

		Is_Dead = true;

		Actor instigator = Scene.Get_Net_ID_Actor(instigator_id);
        Item_Archetype weapon_type = weapon_type_id == -1 ? null : Item_Archetype.Find_By_Type(Type.Find(weapon_type_id));
		
		Log.Write("Pawn " + this.Net_ID + " has died.");

		// Force weapons to finish firing.
		if (m_firing)
		{			
			On_Fire_End();
			m_weapon_slots[m_active_weapon_slot].On_Fire_End();				
			m_firing = false;
		}
		if (m_alt_firing)
		{			
			On_Alt_Fire_End();
			m_weapon_slots[m_active_weapon_slot].On_Alt_Fire_End();				
			m_alt_firing = false;
		}

		// Death effects.
		On_Death(instigator, weapon_type);
		
		// Get rid off the status effects.
		if (m_status_ailment_effect != null && Scene.Is_Spawned(m_status_ailment_effect))
		{
			m_status_ailment_effect.Get_Component().Visible = false;
			m_status_ailment_effect = null;
		}

		//if (Network.Is_Server())
		{
			Despawn();
		}
	}
	
	virtual void On_Death(Actor instigator, Item_Archetype weapon_type)
	{
		// Override in derived classes to spawn death effects etc.
	}

	virtual void Die(Actor instigator, Item_Archetype weapon_type)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Die((instigator != null && Scene.Is_Spawned(instigator)) ? instigator.Net_ID : -1, weapon_type == null ? -1 : weapon_type.GetType().ID);
	}

	// -------------------------------------------------------------------------
	//  RPC functions.
	// -------------------------------------------------------------------------
    virtual void Give_Weapon_Extended(
        Type weapon_type, 
        Type ammo_type,
        Vec4 primary_color, 
        Item_Archetype upgrade_archetype, 
        int[] upgrade_ids, 
        int clip_size,
        int reserve_size,
        int clip_ammo,
        int reserve_ammo
        )
	{
    	rpc(RPCTarget.All, RPCTarget.None) RPC_Give_Weapon_Extended(
            weapon_type.ID, 
            ammo_type == null ? -1 : ammo_type.ID,
            primary_color, 
            upgrade_archetype.GetType().ID,
            upgrade_ids,
            clip_size,
            reserve_size,
            clip_ammo,
            reserve_ammo);
	}
    
	private rpc void RPC_Give_Weapon_Extended(
        int id, 
        int ammo_id, 
        Vec4 primary_color, 
        int upgrade_archetype_id, 
        int[] upgrade_ids, 
        int clip_size, 
        int reserve_size, 
        int clip_ammo, 
        int reserve_ammo)
    {
		Type weapon_type = Type.Find(id);
		if (weapon_type == null)
		{
			return;
		}
		Type ammo_type = (ammo_id == -1 ? null : Type.Find(ammo_id));
		Type upgrade_archetype = Type.Find(upgrade_archetype_id);
		if (upgrade_archetype == null)
		{
			return;
		}
        
        Item_Archetype item_archetype = Item_Archetype.Find_By_Type(upgrade_archetype);

		Weapon weapon = <Weapon>(weapon_type.New());
		weapon.Owner = this;
		weapon.Primary_Color = primary_color;
        
        Log.Write("Giving player weapon " + weapon_type.Name+" with color "+primary_color.X+","+primary_color.Y+","+primary_color.Z+","+primary_color.W+" and "+upgrade_ids.Length()+" upgrades, item archetype is "+(item_archetype != null ? item_archetype.GetType().Name : "None")+".");		

		weapon.Upgrade_Ids = upgrade_ids;
        weapon.Upgrade_Item_Archetype = item_archetype;
        weapon.Upgrade_Tree_Modifiers = (item_archetype != null && item_archetype.Upgrade_Tree != null) ? item_archetype.Upgrade_Tree.Get_Active_Compound_Modifiers(weapon.Upgrade_Ids) : null;

		Safe_Set_Weapon_Slot(weapon.Slot, weapon);
		Safe_Set_Active_Weapon_Slot(weapon.Slot);

        if (ammo_type)
        {
		    Weapon_Ammo ammo = <Weapon_Ammo>(ammo_type.New());
		    m_weapon_slots[m_active_weapon_slot].Change_Ammo_Modifier(ammo);		
        }

        m_weapon_slots[m_active_weapon_slot].Reserve_Size = reserve_size;
        m_weapon_slots[m_active_weapon_slot].Clip_Size = clip_size;
        m_weapon_slots[m_active_weapon_slot].Reserve_Ammo = reserve_ammo;
        m_weapon_slots[m_active_weapon_slot].Clip_Ammo = clip_ammo;

		m_ignore_firing_until_no_input = true;
    }

	private rpc void RPC_Give_Weapon(int id, Vec4 primary_color, bool unlimited_ammo, int[] upgrade_ids, int archetype_id)
	{
		Type weapon_type = Type.Find(id);
		if (weapon_type == null)
		{
			Log.Write("Failed to give pawn weapon with type-id "+id+", type-id is invalid.");
			return;
		}

		Weapon weapon = <Weapon>(weapon_type.New());
		weapon.Owner = this;
		weapon.Primary_Color = primary_color;
        
        Type item_archetype_type = Type.Find(archetype_id);
        Item_Archetype item_archetype = item_archetype_type == null ? null : Item_Archetype.Find_By_Type(item_archetype_type);
        if (!item_archetype)
        {
            item_archetype = weapon.Item_Type;
        }

        Log.Write("Giving player weapon " + weapon_type.Name+" with color "+primary_color.X+","+primary_color.Y+","+primary_color.Z+","+primary_color.W+" and "+upgrade_ids.Length()+" upgrades, item archetype is "+(item_archetype != null ? item_archetype.GetType().Name : "None")+".");		

		weapon.Upgrade_Ids = upgrade_ids;
        weapon.Upgrade_Item_Archetype = item_archetype;
        weapon.Upgrade_Tree_Modifiers = (item_archetype != null && item_archetype.Upgrade_Tree != null) ? item_archetype.Upgrade_Tree.Get_Active_Compound_Modifiers(weapon.Upgrade_Ids) : null;

		if (Owner != null)
		{
			float reserve_multiplier = this.Owner.Stat_Multipliers[Profile_Stat.Ammo_Capacity];
			float clip_multiplier = 1.0f;

			switch (weapon.SubType)
			{
			case Weapon_SubType.Melee:
				{
					clip_multiplier *= this.Owner.Stat_Multipliers[Profile_Stat.Melee_Stamina];
					break;
				}
			case Weapon_SubType.Heavy:
				{
					reserve_multiplier *= this.Owner.Stat_Multipliers[Profile_Stat.Heavy_Weapon_Ammo];
					break;
				}
			case Weapon_SubType.Small_Arms:
				{
					reserve_multiplier *= this.Owner.Stat_Multipliers[Profile_Stat.Small_Arms_Ammo];
					break;
				}
			case Weapon_SubType.Rifle:
				{
					reserve_multiplier *= this.Owner.Stat_Multipliers[Profile_Stat.Rifle_Ammo];
					break;
				}
			case Weapon_SubType.Grenade:
				{
					reserve_multiplier *= this.Owner.Stat_Multipliers[Profile_Stat.Buff_Grenade_Ammo];
					break;
				}
			}

            // Weapon upgrade modifiers.
            if (weapon.Upgrade_Tree_Modifiers != null && 
                weapon.Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.AmmoCapacity] != 0.0f)
            {
                float multiplier = 1.0f + weapon.Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.AmmoCapacity];
                reserve_multiplier *= multiplier;
                clip_multiplier *= multiplier;
            }

			weapon.Reserve_Size = <int>(<float>weapon.Reserve_Size * reserve_multiplier);
			weapon.Clip_Size = <int>(<float>weapon.Clip_Size * clip_multiplier);
		}

		if (unlimited_ammo == true)
		{
			weapon.Has_Unlimited_Ammo = true;
		}
		if (Cheat_Manager.bUnlimitedAmmo)
		{
			weapon.Has_Unlimited_Ammo = true;
		}
		weapon.Fill_Ammo();
		
		Safe_Set_Weapon_Slot(weapon.Slot, weapon);
		Safe_Set_Active_Weapon_Slot(weapon.Slot);

		m_ignore_firing_until_no_input = true;
	}
	
	private rpc void RPC_Give_Weapon_Modifier(int id)
	{
		Type ammo_type = Type.Find(id);		
		//Log.Write("Giving player " + this.Owner.Net_ID + " weapon modifier " + ammo_type.Name);		
		
		Weapon_Ammo ammo = <Weapon_Ammo>(ammo_type.New());
		
		m_weapon_slots[m_active_weapon_slot].Change_Ammo_Modifier(ammo);		
	}
		
	private rpc void RPC_Switch_Weapon_Slot(int slot)
	{		
		//Log.Write("Player " + this.Owner.Net_ID + " changed to weapon slot " + slot);		
		m_active_weapon_slot = slot;
	}	
	  
	private rpc void RPC_Remove_Weapon_Slot(int slot)
	{
		//Log.Write("Remove players " + this.Owner.Net_ID + " weapon from slot " + slot);		
		
		Safe_Set_Weapon_Slot(slot, null);
		Cycle_Weapon_Forward();
	} 
	  
	public virtual void On_Fire_Begin()
	{
	}
	
	public virtual void On_Fire_End()
	{
	}
	
	public virtual void On_Alt_Fire_Begin()
	{
	}
	
	public virtual void On_Alt_Fire_End()
	{
	}

	private rpc void RPC_Begin_Firing()
	{
		if (m_firing == false && m_weapon_slots[m_active_weapon_slot] != null)
		{
			On_Fire_Begin();
			m_weapon_slots[m_active_weapon_slot].On_Fire_Begin(Get_ROF_Multiplier(), Get_Reload_Multiplier());	
			m_firing = true;
		}
	}

	private rpc void RPC_Finish_Firing()
	{
		if (m_firing == true && m_weapon_slots[m_active_weapon_slot] != null)
		{
			On_Fire_End();
			m_weapon_slots[m_active_weapon_slot].On_Fire_End();				
			m_firing = false;
		}
	}
	private rpc void RPC_Begin_Alt_Firing()
	{
		if (m_alt_firing == false && m_weapon_slots[m_active_weapon_slot] != null)
		{
			On_Alt_Fire_Begin();
			m_weapon_slots[m_active_weapon_slot].On_Alt_Fire_Begin(Get_ROF_Multiplier(), Get_Reload_Multiplier());		
			m_alt_firing = true;
		}
	}
	private rpc void RPC_Finish_Alt_Firing()
	{
		if (m_alt_firing == true && m_weapon_slots[m_active_weapon_slot] != null)
		{
			On_Alt_Fire_End();
			m_weapon_slots[m_active_weapon_slot].On_Alt_Fire_End();				
			m_alt_firing = false;
		}
	}
	private rpc void RPC_Begin_Reload()
	{
		if (m_weapon_slots[m_active_weapon_slot] != null)
		{
			m_weapon_slots[m_active_weapon_slot].On_Reload(Get_Reload_Multiplier());				
		}
	}	
	private rpc void RPC_Update_Weapon_State(int clip_ammo, int reserve_ammo, int reloading, float InStamina)
	{
		if (m_weapon_slots[m_active_weapon_slot] != null)
		{
			m_weapon_slots[m_active_weapon_slot].Clip_Ammo = clip_ammo;
			m_weapon_slots[m_active_weapon_slot].Reserve_Ammo = reserve_ammo;
			m_weapon_slots[m_active_weapon_slot].Is_Reloading = reloading;
		}
		this.Stamina = InStamina;
	}

	private rpc void RPC_Set_Weapon_Ammo_State(int slot, int clip, int reserve)
	{
		if (m_weapon_slots[slot] != null)
		{
			m_weapon_slots[slot].Clip_Ammo = clip;
			m_weapon_slots[slot].Reserve_Ammo = reserve;
		}
	}

	private rpc void RPC_Fill_Reserve_Ammo(int index)
	{
		if (m_weapon_slots[index] != null)
		{
			m_weapon_slots[index].Fill_Reserve_Ammo();
		}
	}

	private rpc void RPC_Fill_Ammo(int index)
	{
		if (m_weapon_slots[index] != null)
		{
			m_weapon_slots[index].Fill_Ammo();
		}
	}

	public bool Needs_Ammo_For(Type weapon_type)
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			Weapon w = m_weapon_slots[i];
			if (w != null && w.GetType() == weapon_type)
			{
				if (w.Reserve_Ammo < w.Reserve_Size)
				{
					return true;
				}
			}
		}
		return false;
	}

	public bool Has_Weapon(Type weapon_type)
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			Weapon w = m_weapon_slots[i];
			if (w != null && w.GetType() == weapon_type)
			{
				return true;
			}
		}
		return false;
	}

	// -------------------------------------------------------------------------
	//  Weapon management.
	// -------------------------------------------------------------------------
	private void Safe_Set_Weapon_Slot(int slot, Weapon weapon)
	{
		if (m_weapon_slots[slot] != null)
		{
			// Force weapon to finish firing if it is.
			if (m_active_weapon_slot == slot)
			{			
				if (m_firing)
				{			
					On_Fire_End();
					m_weapon_slots[m_active_weapon_slot].On_Fire_End();				
					m_firing = false;
				}
				if (m_alt_firing)
				{			
					On_Alt_Fire_End();
					m_weapon_slots[m_active_weapon_slot].On_Alt_Fire_End();				
					m_alt_firing = false;
				}
			}
			
			// Unequip old item.
			m_weapon_slots[slot].On_Unequipped();
		}
	
		m_weapon_slots[slot] = weapon;

		// Equip new item.
		if (m_weapon_slots[slot] != null && slot == m_active_weapon_slot)
		{
			m_weapon_slots[slot].On_Equipped();
		}
	}
	
	private void Safe_Set_Active_Weapon_Slot(int slot)
	{
		if (m_weapon_slots[m_active_weapon_slot] != null) 
		{
			// Force weapon to finish firing if it is.
			if (m_active_weapon_slot == slot)
			{			
				if (m_firing)
				{			
					On_Fire_End();
					m_weapon_slots[m_active_weapon_slot].On_Fire_End();				
					m_firing = false;
				}
				if (m_alt_firing)
				{			
					On_Alt_Fire_End();
					m_weapon_slots[m_active_weapon_slot].On_Alt_Fire_End();				
					m_alt_firing = false;
				}
			}
			
			// Unequip old item.
			m_weapon_slots[m_active_weapon_slot].On_Unequipped();
		}
		
		m_active_weapon_slot = slot;
		
		// Equip new item.
		if (m_weapon_slots[m_active_weapon_slot] != null) 
		{
			m_weapon_slots[m_active_weapon_slot].On_Equipped();
		}
	}
	  
	// -------------------------------------------------------------------------
	// Weapon control.
	// -------------------------------------------------------------------------
	virtual void Give_Weapon(Type weapon_type, Vec4 primary_color, bool unlimited_ammo, int[] upgrade_ids, Item_Archetype archetype)
	{
        if (archetype != null)
        {
            Log.Write("RPC GIVE WEAPON = "+archetype.GetType().Name);
        }
		rpc(RPCTarget.All, RPCTarget.None) RPC_Give_Weapon(weapon_type.ID, primary_color, unlimited_ammo, upgrade_ids, archetype == null ? -1 : archetype.GetType().ID);
	}
	virtual void Give_Weapon(Type weapon_type)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Give_Weapon(weapon_type.ID, Vec4(255.0f, 255.0f, 255.0f, 255.0f), false, new int[0], -1);
	}
		
	virtual void Give_Weapon_Modifier(Type ammo_type)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Give_Weapon_Modifier(ammo_type.ID);
	}
	
	virtual void Remove_Weapon(Weapon weapon)
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			Weapon w = m_weapon_slots[i];
			if (w == weapon)
			{	
				rpc(RPCTarget.All, RPCTarget.None) RPC_Remove_Weapon_Slot(i);				
			}
		}
	}
	
	virtual void Remove_Weapon_Slot(int index)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Remove_Weapon_Slot(index);		
	}
	
	virtual void Remove_All_Weapons()
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			Weapon weapon = m_weapon_slots[i];
			if (weapon != null)
			{
				rpc(RPCTarget.All, RPCTarget.None) RPC_Remove_Weapon_Slot(i);			
			}
		}
	}
	
	virtual void Fill_Reserve_Ammo()
	{
		for (int i = 0; i < Weapon_Slot.COUNT; i++)
		{
			Weapon weapon = m_weapon_slots[i];
			if (weapon != null)
			{
				rpc(RPCTarget.All, RPCTarget.None) RPC_Fill_Reserve_Ammo(i);			
			}
		}
	}

	virtual void Fill_Active_Reserve_Ammo()
	{
		Weapon weapon = m_weapon_slots[m_active_weapon_slot];
		if (weapon != null)
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Fill_Reserve_Ammo(m_active_weapon_slot);			
		}
	}
	
	virtual void Fill_Active_Ammo()
	{
		Weapon weapon = m_weapon_slots[m_active_weapon_slot];
		if (weapon != null)
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Fill_Ammo(m_active_weapon_slot);			
		}
	}

	virtual void Set_Active_Weapon_Slot(int slot)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Switch_Weapon_Slot(slot);
	}
	
	virtual Weapon Get_Weapon_Slot(int slot)
	{	
		return m_weapon_slots[slot];
	}
	
	virtual Weapon Get_Active_Weapon()
	{	
		return m_weapon_slots[m_active_weapon_slot];
	}

	virtual int Get_Active_Weapon_Slot()
	{
		return m_active_weapon_slot;
	}
	
	virtual void Cycle_Weapon_Backward()
	{
		for (int i = 1; i < Weapon_Slot.COUNT; i++)
		{
			int slot = m_active_weapon_slot - i;
			if (slot < 0)
			{
				slot = Weapon_Slot.COUNT + slot;
			}
			Weapon weapon = m_weapon_slots[slot];
			if (weapon != null)
			{
				rpc(RPCTarget.All, RPCTarget.None) RPC_Switch_Weapon_Slot(slot);
				break;					
			}
		}
	}
	
	virtual void Cycle_Weapon_Forward()
	{
		for (int i = 1; i < Weapon_Slot.COUNT; i++)
		{
			int slot = (m_active_weapon_slot + i) % Weapon_Slot.COUNT;

			Weapon weapon = m_weapon_slots[slot];
			if (weapon != null)
			{
				rpc(RPCTarget.All, RPCTarget.None) RPC_Switch_Weapon_Slot(slot);
				break;					
			}
		}
	}

	virtual void Tick_Weapon()
	{
		Weapon weapon = m_weapon_slots[m_active_weapon_slot];
		if (weapon != null)
		{
			weapon.On_Tick();
			
			// If local user deal with stats.
			if (this.Owner != null && this.Owner.Is_Local && weapon.Item_Type != null)
			{
				// Update firing stats.
				int bullets_fired = Get_Shots_Fired_Delta();
				if (bullets_fired > 0)
				{
					this.Owner.Local_Profile.Increment_Item_Stat(weapon.Item_Type, Profile_Item_Stat_Type.Shots_Fired, <float>bullets_fired);				
				}
			}
		}

		Tick_Throttle_Damage();
		Tick_Ailment();
	}
	
	virtual bool Is_Firing()
	{
		return m_firing;
	}

	virtual bool Is_Alt_Firing()
	{
		return m_alt_firing;
	}
	
	virtual bool Is_Interact_Locked()
	{
        if (m_firing || m_alt_firing)
        {
            return true;
        }

		Weapon weapon = m_weapon_slots[m_active_weapon_slot];
		if (weapon != null)
		{
			return weapon.Is_Interact_Locked();
		}

		return false;
	}

	// -------------------------------------------------------------------------
	// Inventory control. None of this is synced, as its primarily used for 
	// cosmetic items, that we already have access to via user's Profile.
	// -------------------------------------------------------------------------
	private rpc void RPC_Give_Item(Item item)
	{
		m_item_slots[item.Equip_Slot] = item;
	}

	private rpc void RPC_Remove_Item(int slot)
	{
		m_item_slots[slot] = null;
	}

	private rpc void RPC_Remove_All_Items()
	{
		for (int i = 0; i < Item_Slot.COUNT; i++)
		{
			m_item_slots[i] = null;
		}
	}

	virtual void Give_Item(Item item)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Give_Item(item);
	}
	
	virtual void Remove_Item(Item item)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Remove_Item(item.Equip_Slot);
	}
	
	virtual void Remove_All_Items()
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Remove_All_Items();

		//Log.Write("Player " + this.Owner.Net_ID + " had all items removed.");
	}
	
	virtual Item Get_Item_Slot(int slot)
	{	
		return m_item_slots[slot];
	}

    public virtual string Get_Hud_Head_Animation()
    {
        return "";
    }
	
	// -------------------------------------------------------------------------
	// Attacking
	// -------------------------------------------------------------------------
	virtual void Update_Weapon_State()
	{
		int		clip_ammo		= m_weapon_slots[m_active_weapon_slot].Clip_Ammo;
		int		reserve_ammo	= m_weapon_slots[m_active_weapon_slot].Reserve_Ammo;
		bool	reloading		= m_weapon_slots[m_active_weapon_slot].Is_Reloading;

		rpc(RPCTarget.All, RPCTarget.Local) RPC_Update_Weapon_State(clip_ammo, reserve_ammo, reloading, Stamina);
	}
	
	virtual void Set_Weapon_Ammo_State(int slot, int clip, int reserve)
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Set_Weapon_Ammo_State(slot, clip, reserve);
	}

	virtual void Begin_Fire()
	{
		Update_Weapon_State();
		rpc(RPCTarget.All, RPCTarget.None) RPC_Begin_Firing();
	}
	
	virtual void Finish_Fire()
	{
		Update_Weapon_State();
		rpc(RPCTarget.All, RPCTarget.None) RPC_Finish_Firing();
	}
	
	virtual void Begin_Alt_Fire()
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Begin_Alt_Firing();
	}
	
	virtual void Finish_Alt_Fire()
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Finish_Alt_Firing();
	}
	
	virtual void Begin_Reload()
	{
		Update_Weapon_State();
		rpc(RPCTarget.All, RPCTarget.None) RPC_Begin_Reload();
	}
		    
	virtual bool Is_Incapacitated()
    {
        return false;
    }

	// -------------------------------------------------------------------------
	// Multipliers - these need to go elsewhere really.
	// -------------------------------------------------------------------------
	public virtual float Get_XP_Multiplier()
	{
		return 1.0f;
	}
	
	public virtual float Get_Speed_Multiplier()
	{
		return 1.0f;
	}
	
	public virtual float Get_ROF_Multiplier()
	{
		return 1.0f;
	}

	public virtual float Get_Reload_Multiplier()
	{
		return 1.0f;
	}

	public virtual float Get_Price_Multiplier()
	{
		return 1.0f;
	}
	
	public virtual float Get_Damage_Multiplier()
	{
		return 1.0f;
	}

	// -------------------------------------------------------------------------
	// Movement.
	// -------------------------------------------------------------------------
	abstract void Move_Towards(Vec3 pos);
	abstract void Turn_Towards(Direction dir);	
}