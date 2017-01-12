// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;

public class Love_Pylon_Beam
{
    public int Target_Net_ID;
    public Muzzle_Flash Beam_Effect;
    public float Last_Beam_Fire_Timer;
}

[
	Placeable(true), 
	Name("Love Pylon"), 
	Description("Patta patta patta.") 
]
public class Love_Pylon : Movable 
{
    public float Hud_Element_Alpha = 1.0f;

	private float SCAN_RANGE = 80.0f;

    private float[] m_upgrade_modifiers = null;

    private serialized(1) int[] m_target_ids = new int[0];
    private Love_Pylon_Beam[] m_beams = new Love_Pylon_Beam[0];
    
	private Projectile_Weapon m_weapon = null;

	private Audio_Channel m_fire_sound_channel;
    
    override void Set_Upgrade_Modifiers(float[] modifiers)
	{
        m_upgrade_modifiers = modifiers;
	}
    
	public bool Is_Valid_Target(Pawn other)
	{
        // Nothing is valid when picked up.
        if (Pickup_Pawn_ID != -1)
        {
            return false;
        }

        // Humans only.
        if ((<Human_Player> other) == null)
        {
            return false;
        }

		// Not if on other team.
		if (other.Owner != null && other.Owner.Team_Index != Team_ID)
		{
			return false;
		}

        // Ignore healed players.
        if (other.Health >= other.Max_Health)
        {
            return false;
        }
        
		return true;
	}

	public float Get_Health_Scalar()
	{
		return Health / Max_Health;
	}

	public float Get_Ammo_Scalar()
	{
		if (m_weapon == null)
		{
			Setup_Weapon();
		} 
        
		return m_weapon.Clip_Ammo / <float>m_weapon.Clip_Size;
	}
    
	public void Setup_Weapon()
	{
		m_weapon = new Weapon_Machine_Love_Pylon();
        m_weapon.Upgrade_Tree_Modifiers = m_upgrade_modifiers;
		
		// Health modifier.
		if (this.Owner != null)
		{
			Health *= this.Owner.Stat_Multipliers[Profile_Stat.Turret_Damage];
			Max_Health = Health;
		}

		// Ammo modifier.
		if (this.Owner != null)
		{
			m_weapon.Clip_Size *= this.Owner.Stat_Multipliers[Profile_Stat.Turret_Ammo];
		}

		m_weapon.Owner = this;
		m_weapon.Fill_Ammo();
	}
	
	protected override void On_Destroy()
	{
		if (m_fire_sound_channel != null)
		{
			m_fire_sound_channel.Stop();
			m_fire_sound_channel = null;
		}
	}

    public rpc void RPC_Update_Target_Ids(int[] target_ids)
    {
        m_target_ids = target_ids;
    }

	public override void On_Update_State()
	{
		if (m_weapon == null)
		{
			Setup_Weapon();
		} 

		// Tick weapon.
		m_weapon.On_Tick();

        Vec2 muzzle_position = this.Position.XY + m_weapon.Muzzle_Position[0];

        if (Network.Is_Server())
        {
            // Scan for any targets in our FOV.
            Actor[] potential_targets = Scene.Find_Actors_In_Radius(
                typeof(Pawn),
                SCAN_RANGE,
                Vec3(muzzle_position, 0.0f)
            );

            bool bModified = false;
                
            int[] removed_ids = new int[0];

            foreach (int id in m_target_ids)
            {
                bool bExists = false;

                foreach (Pawn p in potential_targets)
                {
                    if (p.Net_ID == id && Is_Valid_Target(p))
                    {
                        bExists = true;
                        break;
                    }
                }

                if (!bExists)
                {
                    removed_ids.AddLast(id);
                }
            }

            foreach (int id in removed_ids)
            {
                m_target_ids.Remove(id);
                bModified = true;
            }

            foreach (Pawn p in potential_targets)
            {
                if (!m_target_ids.Contains(p.Net_ID))
                {
                    if (Is_Valid_Target(p))
                    {
                        m_target_ids.AddLast(p.Net_ID);
                        bModified = true;
                    }
                }
            }

            if (bModified)
            {
    			rpc(RPCTarget.All, RPCTarget.None) RPC_Update_Target_Ids(m_target_ids);
            }
        }
        
        // Remove any targets no longer in list.
        foreach (Love_Pylon_Beam beam in m_beams)
        {
            if (!m_target_ids.Contains(beam.Target_Net_ID))
            {
                Log.Write("Removed love pylon beam with id " + beam.Target_Net_ID);

                if (beam.Beam_Effect && Scene.Is_Spawned(beam.Beam_Effect))
                {
                    beam.Beam_Effect.Dispose();
                }
                beam.Beam_Effect = null;
                m_beams.Remove(beam);
                break;
            }
        }

        // Add new targets in list.
        foreach (int net_id in m_target_ids)
        {
            Actor actor = Scene.Get_Net_ID_Actor(net_id);
            bool bFound = false;

            foreach (Love_Pylon_Beam beam in m_beams)
            {
                if (beam.Target_Net_ID == net_id)
                {
                    bFound = true;
                    break;
                }
            }

            if (!bFound && actor != null)
            {
                Log.Write("Added love pylon beam with id " + net_id);

                Love_Pylon_Beam beam = new Love_Pylon_Beam();
                beam.Target_Net_ID = net_id;
                beam.Beam_Effect = null;

                m_beams.AddLast(beam);
            }
        }

        // Update beam directions.
        Love_Pylon_Beam needs_effect_beam = null;
        foreach (Love_Pylon_Beam beam in m_beams)
        {
            Actor actor = Scene.Get_Net_ID_Actor(beam.Target_Net_ID);
            if (actor != null)
            {
                if (beam.Beam_Effect == null || !beam.Beam_Effect.Is_Active())
                {
                    if (needs_effect_beam == null || beam.Last_Beam_Fire_Timer < needs_effect_beam.Last_Beam_Fire_Timer)
                    {
                        needs_effect_beam = beam;
                    }
                }

                if (beam.Beam_Effect)
                {
                    beam.Beam_Effect.Direction_Offset = (muzzle_position - actor.World_Bounding_Box.Center()).Direction() - Math.HalfPI;
                }
            }
        }

        if (needs_effect_beam != null && m_weapon.Begin_Fire_Spawn())
        {
            needs_effect_beam.Beam_Effect = m_weapon.Spawn_Muzzle_Flash(0);
            needs_effect_beam.Last_Beam_Fire_Timer = Time.Get_Ticks();
            m_weapon.End_Fire_Spawn(1);
        }

        if (m_beams.Length() > 0)
        {
            if (m_fire_sound_channel == null || !m_fire_sound_channel.Is_Playing)
            {
                m_fire_sound_channel = Audio.Play3D("sfx_objects_weapons_love_thrower_fire", this.Position);
            }
        }
        else
        {
		    if (m_fire_sound_channel != null)
		    {
			    m_fire_sound_channel.Stop();
			    m_fire_sound_channel = null;
		    }
        }
        
		// Out of ammo, or user has left? Destroy.
		if (Network.Is_Server())
		{
			bool should_destroy = false;

			// No Ammo.
			if (!m_weapon.Has_Ammo())
			{
				should_destroy = true;
			}

			NetUser user = Get_Imposing_As();
			if (user != null)
			{			
				// Owner changed team.
				if (user.Team_Index != Team_ID)
				{
					should_destroy = true;
				}
			}

			// Owner left game.
			else if (Owner_ID >= 0)
			{
				should_destroy = true;
			}

			if (should_destroy)
			{
				rpc(RPCTarget.All, RPCTarget.None) RPC_Destroy();
				return;
			}
		}
    }

	Love_Pylon()
	{
		Pickup_Sound		= "sfx_objects_destroyable_move_start";
		Pickup_Effect		= "";
		Drop_Sound			= "sfx_objects_destroyable_move_end";
		Drop_Effect			= "dust_puff";
		Destroy_Sound		= "sfx_objects_destroyable_poof";
		Destroy_Effect		= "turret_destroy";

		Bounding_Box		= Vec4(0, 0, 48, 48);
		//Collision_Box 		= Vec4(11, 17, 26, 24);
		Collision_Box 		= Vec4(15, 22, 18, 16);
		
		//sprite.Depth_Bias	= 64;
		sprite.Frame		= "actor_love_pylon_0";

        Max_Health = 150.0f;
	    Health = Max_Health;
	}
}