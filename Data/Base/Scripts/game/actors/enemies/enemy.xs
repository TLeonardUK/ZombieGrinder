// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.pawn;
 
public interface EnemyAttackable
{
	// Dosen't do anything, just marks a class as attackable by enemies.
}
 
[
	Placeable(false), 
	Name("Enemy"), 
	Description("Base class for enemies.") 
]
public class Enemy : Pawn
{  	
	// -------------------------------------------------------------------------
	// Non-replicated variables.
	// -------------------------------------------------------------------------
	protected Direction  m_old_direction 	= Direction.S;
	protected bool 		 m_old_is_idle 		= false;
	protected bool 		 m_is_idle 			= false;
	protected Vec3		 m_old_position;

	// If the enemy is currently a risk to any AI.
	public float		 AI_RiskFactor		= 1.0f;

	// If set the enemy was spawned through magic - give em a smoke puff and a sound fx.
	public serialized(600) bool Spawned_By_Magic	= false; 
	private bool m_spawned_magic_effect = false;

	// Arena this enemy is constrained to for the purposes of finding targets etc.
	public Dungeon_Arena Parent_Arena = null;

	const float STAMINA_RECHARGE_RATE = 25.0f;
	
	// -------------------------------------------------------------------------
	// Magic spawn/despawn.
	// -------------------------------------------------------------------------
	public void Magic_Despawn()
	{
		rpc(RPCTarget.All, RPCTarget.None) RPC_Magic_Despawn();
	}

	public rpc void RPC_Magic_Despawn()
	{
		FX.Spawn_Untracked("enemy_spawn", Vec3(this.World_Bounding_Box.Center(), 0.0f), 0.0f, Layer + 1);
		if (Network.Is_Server())
		{
			this.Despawn();
		}
	}

	// -------------------------------------------------------------------------
	// Updates some basic enemy state.
	// -------------------------------------------------------------------------
	public void Base_Update()
	{
		float delta = Time.Get_Delta();

		// Spawn magic effect?
		if (Spawned_By_Magic)
		{
			if (m_spawned_magic_effect == false)
			{
				Log.Write("Spawning magic spawn effect!");
				FX.Spawn_Untracked("enemy_spawn", Vec3(this.World_Bounding_Box.Center(), 0.0f), 0.0f, Layer + 1);
				m_spawned_magic_effect = true;
			}
		}
		
		// Recharge stamina for stamina based weapons.
		float recharge_rate = STAMINA_RECHARGE_RATE;
        Weapon active_weapon = Get_Active_Weapon();
        if (active_weapon != null && active_weapon.Stamina_Based)
        {
            recharge_rate = active_weapon.Recharge_Rate;
        }
		Stamina = Math.Min(Max_Stamina, Stamina + (recharge_rate * Time.Get_Delta_Seconds()));

		// Idle state.
		Vec3 position_delta = Position - m_old_position;
		m_is_idle = (position_delta.X == 0 && position_delta.Y == 0);
    
		// Store old state.
		m_old_direction = Current_Direction;
		m_old_is_idle 	= m_is_idle;
		m_old_position 	= Position;
	}
	
	// -------------------------------------------------------------------------
	// Movement.
	// -------------------------------------------------------------------------
	override void Move_Towards(Vec3 pos)
	{
		MoveTo(pos);
	}
	
	override void Turn_Towards(Direction dir)
	{
		Current_Direction = dir;
	}

	// -------------------------------------------------------------------------
	//  Replication.
	// -------------------------------------------------------------------------
	replicate(ReplicationPriority.High)
	{
		Position
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Interpolate_Smoothed;
		}
		Current_Direction
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Absolute;
		}
		Health
		{
			Owner = ReplicationOwner.ObjectOwner;
			Mode  = ReplicationMode.Absolute;
		} 
	}
}
