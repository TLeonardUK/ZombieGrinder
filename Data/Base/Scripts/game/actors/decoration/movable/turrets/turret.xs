// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;
using game.weapons.machines.weapon_machine_turret;
using game.weapons.machines.weapon_machine_explosive_turret;
using game.weapons.machines.weapon_machine_incendiary_turret;
using game.weapons.machines.weapon_machine_freeze_turret;

[
	Placeable(true), 
	Name("Turret"), 
	Description("Patta patta patta.") 
]
public class Turret : Movable 
{
    public float Hud_Element_Alpha = 1.0f;

	private const float SCAN_TURN_INTERVAL	= 0.25f;
	private		  float SCAN_RANGE			= 200.0f;
	private       float SCAN_FOV			= Math.PI * 0.15f;
	
	private		  float TRACK_RANGE			= 300.0f;

	private Direction Current_Direction = Direction.S;
	private float m_turn_timer = 0.0f;

	private Pawn m_target = null;

	private Weapon m_weapon = null;
	public serialized(1) string Ammo_Modifier = "";
	private string m_ammo_variant = "rocket";

	private Pawn[] m_potential_targets = new Pawn[0];
	private Pawn m_checking_target = null;

    private float[] m_upgrade_modifiers = null;
	
	override void Set_Ammo_Modifier(Weapon_Ammo ammo)
	{
		if (ammo != null)
		{
			Ammo_Modifier = ammo.Effect_Modifier;
		}
		else
		{
			Ammo_Modifier = "";
		}
	}

    override void Set_Upgrade_Modifiers(float[] modifiers)
	{
        m_upgrade_modifiers = modifiers;
	}

	public bool Is_Valid_Target(Pawn other)
	{
		// Not if its our owner.
		if (other.Owner != null && other.Owner.Net_ID == Owner_ID)
		{
			return false;
		}

		// Not if on the same team.
		if (other.Owner != null && other.Owner.Team_Index == Team_ID)
		{
			return false;
		}

		// Ignore decapitated zombies / incapacitated humans.
		if (!other.Can_Target())
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

	public override Direction Get_Direction()
	{
		return Current_Direction;
	}
	
	public void Setup_Weapon()
	{
		if (Ammo_Modifier == "Incendiary")
		{		
			m_weapon = new Weapon_Machine_Incendiary_Turret();
            m_weapon.Upgrade_Tree_Modifiers = m_upgrade_modifiers;
			m_ammo_variant = "fire";

			SCAN_RANGE = 100.0f;
			TRACK_RANGE = 150.0f;
		}
		else if (Ammo_Modifier == "Explosive")
		{
			m_weapon = new Weapon_Machine_Explosive_Turret();
            m_weapon.Upgrade_Tree_Modifiers = m_upgrade_modifiers;
			m_ammo_variant = "rocket";
		}
		else if (Ammo_Modifier == "Freeze")
		{
			m_weapon = new Weapon_Machine_Freeze_Turret();
            m_weapon.Upgrade_Tree_Modifiers = m_upgrade_modifiers;
			m_ammo_variant = "fire";

			SCAN_RANGE = 100.0f;
			TRACK_RANGE = 150.0f;
		}
		else 
		{			
			m_weapon = new Weapon_Machine_Turret();
            m_weapon.Upgrade_Tree_Modifiers = m_upgrade_modifiers;
			m_ammo_variant = "uzi";
		}
		
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
		m_weapon.On_Fire_End();
	}

	public override void On_Update_State()
	{
		if (m_weapon == null)
		{
			Setup_Weapon();
		} 

		float delta_t = Time.Get_Delta_Seconds();
		string color_variant = (Team_ID >= 0 ? (Team_ID % 4) : 4) + 1; // 1-4=team colors 5=server color
		string ammo_variant = m_ammo_variant;

		// Tick weapon.
		m_weapon.On_Tick();

		// Scan for target.
		if (Pickup_Pawn_ID == -1)
		{
			bool target_changed_this_frame = false;

			if (m_target == null)
			{
				// Turn every so often.
				m_turn_timer += delta_t;
				if (m_turn_timer >= SCAN_TURN_INTERVAL)
				{
					Current_Direction = (Current_Direction + 1) % 8;
					m_turn_timer = 0.0f;
				}

				// Check all potential targets.
				if (m_potential_targets.Length() > 0 || m_checking_target != null)
				{
					if (m_checking_target != null)
					{
						// No LOS collision? We have a target!
						if (!los_checker.Is_Colliding())
						{
							Log.Write("[Turret] Found target with open LOS, targetting.");

							Audio.Play3D("sfx_objects_turrets_target_acquired", Position);

							m_target = m_checking_target;
							m_weapon.On_Fire_Begin(1.0f, 1.0f);
							m_potential_targets.Clear();
							m_checking_target = null;
							target_changed_this_frame = true;
						}
						else
						{
							//Log.Write("[Turret] Target has blocking LOS, ignoring.");

							m_checking_target = null;
						}
					}

					if (m_potential_targets.Length() > 0)
					{
						//Log.Write("[Turret] Checking next potential target.");

						Pawn current_target = m_potential_targets.RemoveLast();
						m_checking_target = current_target;
					}
				}

				// No targets left, scan for some.
				else
				{
					// Scan for any targets in our FOV.
					Actor[] potential_targets = Scene.Find_Actors_In_FOV(
						typeof(Pawn), 
						SCAN_RANGE,
						this.Collision_Center, 
						SCAN_FOV, 
						Direction_Helper.Vectors[Current_Direction]
					);
					if (potential_targets.Length() > 0)
					{
						foreach (Pawn p in potential_targets)
						{
							if (Is_Valid_Target(p))
							{
								m_potential_targets.AddLast(p);
							}
						}

						//if (m_potential_targets.Length() > 0)
						//{
						//	Log.Write("[Turret] Found "+m_potential_targets.Length()+" potential targets. Starting LOS checks.");
						//}
					}
				}
			}

			// Update LOS checker.
			Pawn aiming_at = m_target;
			if (aiming_at == null)
			{
				aiming_at = m_checking_target;
			}
			if (aiming_at != null && Scene.Is_Spawned(aiming_at))
			{			
				Vec3 muzzle_position = this.Position + Vec3(m_weapon.Muzzle_Position[Current_Direction], 0.0f);

				los_checker.Area = Vec4
				(
					muzzle_position.X,
					muzzle_position.Y,
					aiming_at.Collision_Center.X,
					aiming_at.Collision_Center.Y
				);
				los_checker.Enabled = true;
			}
			else
			{
				los_checker.Enabled = false;
			}

			// Track target.
			if (m_target != null)
			{
				if (Scene.Is_Spawned(m_target))
				{
					Vec3 heading = (m_target.Center - this.Center);

					// Out of range?
					if (heading.Length() > TRACK_RANGE || (los_checker.Is_Colliding() && !target_changed_this_frame))
					{
						Log.Write("[Turret] Target out of range, or environment occluding our LOS.");

						Audio.Play3D("sfx_objects_turrets_target_lost", Position);
						m_target = null;
						m_weapon.On_Fire_End();
					}
					else
					{
						// Face them.
						Current_Direction = Direction_Helper.Calculate_Direction(heading.X, heading.Y, Current_Direction);
					}
				}
				else
				{
					Log.Write("[Turret] Target has been despawned.");

					Audio.Play3D("sfx_objects_turrets_target_lost", Position);
					m_target = null;
					m_weapon.On_Fire_End();
				}
			}
		}
		else
		{
			if (m_target != null)
			{	
				Log.Write("[Turret] Removing target due to pickup.");
	
				Audio.Play3D("sfx_objects_turrets_target_lost", Position);
				m_target = null;
				m_weapon.On_Fire_End();
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

		// Update sprites.
		sprite.Frame = "";
		shadow_sprite.Frame = "";
		sprite.Animation = "actor_turrets_" + ammo_variant + "_turret_" + color_variant + "_" + Direction_Helper.Mnemonics[Current_Direction];
		shadow_sprite.Animation = "actor_turrets_" + ammo_variant + "_turret_" + color_variant + "_" + Direction_Helper.Mnemonics[Current_Direction];
	}

	Turret()
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
		sprite.Frame		= "actor_turrets_uzi_turret_3";

        Max_Health = 70.0f;
	    Health = Max_Health;
	}

	// -------------------------------------------------------------------------
	//  LOS check.
	// -------------------------------------------------------------------------
	components
	{
		CollisionComponent los_checker
		{
			Shape 			= CollisionShape.Line;
			Area 			= Vec4(0, 0, 0, 0);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Environment;
			Enabled			= false;
		}
	}
}