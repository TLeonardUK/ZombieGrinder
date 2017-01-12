// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.actors.decoration.movable.movable;

[
	Placeable(true), 
	Name("Enemy Barricade"), 
	Description("Base class for all items that can either be picked up or bought off walls.") 
]
public class Enemy_Barricade : Actor, Interactable, EnemyAttackable
{
	[  
		Name("Rebuildable"),
		Description("If set to true this barricade can be rebuilt.")
	]
 	public serialized(1) bool Rebuildable = true;
	
	[ 
		Name("Cost"),
		Description("Coin cost per rebuild unit.")
	]
	public serialized(1) int Rebuild_Cost = 100;
	
	[ 
		Name("Rebuild Health"),
		Description("How much health to give on rebuild.")
	]
	public serialized(1) float Rebuild_Health = 10.0f;
	
	[ 
		Name("Max Health"),
		Description("Maximum health this barricade starts at.")
	]
	public serialized(1) float Max_Health = 100.0f;
	
	[ 
		Name("Health"),
		Description("Current barricade health.")
	]
	public serialized(1) float Health = 100.0f;
	
	[ 
		Description("Fraction of total height to use for collision height")
	]
	public serialized(1) float Collision_Height_Fraction = 0.25;
	
	[ 
		EditType("Animation"),
		Description("Fraction of total height to use for collision height")
	]
	public serialized(1) string Animation_Name = "actor_barricade_zombie_barricade_1_destroy";
	
	Vec4 DAMAGE_AREA_BUFFER = new Vec4(-2.0f, -2.0f, 4.0f, 4.0f);

	private int Team_Index = 0;

	private float m_last_health = 100.0f;

	Enemy_Barricade()
	{
		Bounding_Box = Vec4(0, 0, 32, 32);
	}

	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return Rebuildable ? Rebuild_Cost : 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#rebuild_prompt");
	}
	
	public override bool Can_Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			if (Rebuildable && Health < Max_Health)
			{
				return true;
			}
		}
		if (Health <= 0.0f)
		{
			if (collision.Is_Colliding())
			{
				return false;
			}
		}
		return false;
	}

	public override void Interact(Pawn other)
	{
		rpc(RPCTarget.Server, RPCTarget.None) RPC_Request_Trigger(other.Net_ID);
	}
	
	private rpc void RPC_Request_Trigger(int id)
	{
		Pawn other = <Pawn>Scene.Get_Net_ID_Actor(id);

		if (Network.Is_Server() && other != null)
		{
			if ((<Interactable>this).Can_Interact(other)) // TODO: Fix compiler, this cast should be redundent.
			{
				Health = Math.Min(Health + Rebuild_Health, Max_Health);
			}
		}

		Audio.Play3D("sfx_objects_barricades_zombie_build", this.Center);
		FX.Spawn_Untracked("barricade_buy", this.Center, 0, this.Layer, null, <int>Rebuild_Health);
	}

	private void Update_Frame()
	{
		float delta = 1.0f - (Health / Max_Health);
		int frame_count = Canvas.Get_Animation_Frame_Count(Animation_Name);
		int frame = Math.Clamp(<int>((frame_count - 1) * delta), 0, frame_count - 1);
		sprite.Frame = Canvas.Get_Animation_Frame(Animation_Name, frame);
	}
	
	default state Alive
	{
		event void On_Enter()
		{
			collision.Type	= CollisionType.Solid;		
			collision.Group	= CollisionGroup.Environment;
			
			Vec4 bb = this.Bounding_Box;
			float height = bb.Height * Collision_Height_Fraction;
			collision.Area = Vec4(bb.X, bb.Y + (bb.Height - height), bb.Width, height);
			damage_sensor.Area = collision.Area + DAMAGE_AREA_BUFFER;

			sprite.Depth_Bias = bb.Y + bb.Height;// + (bb.Height * 0.25f);

			Update_Frame();
			m_last_health = Health;
		}
		event void On_Editor_Property_Change()
		{
			Update_Frame();
		}
		event void On_Tick()
		{
			if (Health != m_last_health)
			{			
				Update_Frame();
				m_last_health = Health;

				if (Health > m_last_health)
				{
					Audio.Play3D("sfx_objects_destroyable_poof", this.Center);
					FX.Spawn_Untracked("barricade_hit", this.Center, 0, this.Layer);
				}
			}
			if (Health <= 0.0f)
			{
				change_state(Dead);
			}
		}
	} 
	
	state Dead
	{
		event void On_Enter()
		{
			collision.Type	= CollisionType.Non_Solid;		
			collision.Group	= CollisionGroup.Volume;

			Update_Frame();
			
			Audio.Play3D("sfx_objects_destroyable_poof", this.Center);
			FX.Spawn_Untracked("barricade_destroy", this.Center, 0, this.Layer);
			
			Log.Write("Enemy barricade died!");
		}
		event void On_Tick()
		{
			if (Health > 0.0f)
			{
				Log.Write("Enemy barricade was resurrected!");
				change_state(Alive);
			}
		}
	}
	
    void Reduce_Health(float amount)
    {
        On_Damage(damage_sensor, null, this.Position, 0.0f, amount, CollisionDamageType.Melee, 0, 0, Weapon_SubType.Melee, null);
    }

	event bool On_Damage(
		CollisionComponent component, 
		Actor instigator, 
		Vec3 position, 
		float dir, 
		float damage, 
		CollisionDamageType type,
		int ricochet_count,
		int penetration_count,
		Weapon_SubType weapon_subtype,
        Item_Archetype weapon_item)
	{
		// Still alive?
		if (Health <= 0.0f)
		{
			return false;
		}

		if (Network.Is_Server())
		{
			// Make sure turrets etc don't shot themselves.
			if (instigator == this)
			{
				return false;
			}

			if (instigator != null && Team_Index <= 0 && Team_Index == instigator.Get_Team_Index())
			{
				return false;
			}
			
			// Scale by instigators damage multiplier.
			Health = Math.Max(0.0f, Health - damage);
			Log.Write("Barricade was damaged, health is now "+Health+".");

			return true;
		}

		return true;
	}

	// Force replication as network object.
	replicate(ReplicationPriority.Low)
	{
		Health
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}		
	}
	
	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_barricade_zombie_barricade_1_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		serialized(1) CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 32, 32);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Environment;
			Collides_With	= CollisionGroup.All;
			Blocks_Path_Spawns	= true;
		}
		CollisionComponent damage_sensor
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 32, 32);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Damage;
		}
	}
}
