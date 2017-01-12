// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

[
	Placeable(true), 
	Name("Mounted Gun"), 
	Description("Stick on a wall and pew pew pew.") 
]
public class Mounted_Gun : Actor, IDungeonRoomHibernatedObject
{
	// Type of weapon to fire.
	public serialized(1) string Weapon_Type = "Weapon_Flamethrower";

	// Time between firing rounds.
	public serialized(1) float Spawn_Interval = 1.0f;

	// Duration of firing round.
	public serialized(1) float Spawn_Duration = 3.0f;
	
	// Staggered offset for spawning.
	public serialized(1) float Spawn_Offset = 0.0f;
	
	// Firing direction
	public serialized(1) Direction Fire_Direction = Direction.S;
	
	// Firing if active, otherwise not .... duh
	public serialized(600) bool Is_Active = true;

	private bool m_firing = false;
	private float m_state_time_remaining = 0.0f;
	private bool m_weapon_firing = false;
	private Weapon m_weapon = null;

	public override Direction Get_Direction()
	{
		return Fire_Direction;
	}

	public override int Get_Team_Index()
	{
		return 1337;
	}

	Mounted_Gun()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	 
	public override void On_Hibernate()
	{
		if (m_weapon_firing)
		{
			m_weapon.On_Fire_End();
			m_weapon_firing = false;
		}
	}

	public override void On_Wakeup()
	{
	}
	
	public override void On_Trigger()
	{
		Is_Active = !Is_Active;
	}

	default state Idle
	{
		event void On_Enter()
		{
			Type weapon_type = Type.Find(Weapon_Type);
			if (weapon_type == null)
			{
				Hibernate();
				return;
			}

			m_weapon = <Weapon>weapon_type.New();
			m_weapon.Has_Unlimited_Ammo = true;
			m_weapon.Owner = this;
			m_weapon.Ignore_Spawn_Collision = true;
			m_state_time_remaining = Spawn_Offset;

			Vec2 bb_center = this.Bounding_Box.Center();
			float half_size = this.Bounding_Box.Width * 0.4f;

			m_weapon.Muzzle_Position = {
				bb_center + Vec2(0.0f, half_size),
				bb_center ,
				bb_center + Vec2(half_size, 0.0f),
				bb_center,
				bb_center + Vec2(0.0f, -half_size),
				bb_center,
				bb_center + Vec2(-half_size, 0.0f),
				bb_center
			};  

			Hibernate();
		}

		event void On_Tick()
		{
			if (!Is_Active)
			{
				if (m_weapon_firing)
				{
					m_weapon.On_Fire_End();
					m_weapon_firing = false;
				}
			}
			else
			{
				if (Network.Is_Server())
				{
					m_state_time_remaining -= Time.Get_Delta_Seconds();

					if (m_firing == true)
					{
						if (m_state_time_remaining <= 0.0f)
						{
							m_state_time_remaining = Spawn_Interval;
							m_firing = false;
						}
					}
					else
					{
						if (m_state_time_remaining <= 0.0f)
						{
							m_state_time_remaining = Spawn_Duration;
							m_firing = true;
						}
					}
				}

				if (m_firing)
				{
					m_weapon.Fill_Ammo();
					m_weapon.On_Tick();

					if (!m_weapon_firing)
					{
						m_weapon.On_Fire_Begin(1.0f, 1.0f);
						m_weapon_firing = true;
					}
				}
				else
				{
					if (m_weapon_firing)
					{
						m_weapon.On_Fire_End();
						m_weapon_firing = false;
					}
				}
			}
		}
	}
	
	replicate(ReplicationPriority.Normal)
	{	
		m_firing
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
		Is_Active
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(450) SpriteComponent sprite
		{
			Frame 			= "actor_projectile_spawner_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= true; 
			Editor_Visible 	= true;
		}
	}
}