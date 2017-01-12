// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Dungeon_Destroyable_Door"), 
	Description("When hit with explosive rounds this door explodes.") 
]
public class Dungeon_Destroyable_Door : Actor
{
	[ 
		EditType("Animation"),
		Description("Animation to play when opening. Closing is the same in reverse.")
	]
	public serialized(1) string Open_Animation = "actor_door_dungeon_up_destroyable_open";
	
	[ 
		Description("Effect spawned when destroyed.")
	]
	public serialized(1) string Explode_Effect = "explosive_destroy";
	
	[ 
		Description("True if this door is currently open or not.")
	]
	public serialized(1) bool Is_Open = false;

	[ 
		Description("Fraction of total height to use for collision height")
	]
	public serialized(101) float Collision_Height_Fraction = 0.25;

	private bool m_last_open_state = false;

	Dungeon_Destroyable_Door()
	{
		Bounding_Box	= Vec4(0, 0, 48, 48);
		collision.Area 	= Vec4(0, 0, 48, 48);
		Store_Initial_State();
	}

	private void Store_Initial_State()
	{		
		m_last_open_state = Is_Open;
		Update_State();
	}

	private void Update_State()
	{
		if (Is_Open)
		{
			collision.Type	= CollisionType.Non_Solid;		
			collision.Group	= CollisionGroup.Volume;
		}
		else
		{
			collision.Type	= CollisionType.Solid;		
			collision.Group	= CollisionGroup.Environment;
			collision.Collides_With	= CollisionGroup.Player | CollisionGroup.Enemy;
		}				
		
		collision.Area = Vec4(16.0f, 16.0f, 16.0f, 16.0f);
		damage_sensor.Area = Vec4(16.0f, 16.0f, 16.0f, 16.0f);
		
		sprite.Animation = Open_Animation;
		sprite.Animation_Speed = (Is_Open) ? 1.0f : -1.0f;
		sprite.Frame = "";
		sprite.Depth_Bias = this.Bounding_Box.Height;//collision.Area.Y + (collision.Area.Height * 0.5f);

		Log.Write("Playing animation (Is_Open="+Is_Open+") "+sprite.Animation+" with speed "+sprite.Animation_Speed);
	}

	default state Idle
	{
		event void On_Enter()
		{
			Store_Initial_State();
		}
		event void On_Editor_Property_Change()
		{
			Store_Initial_State();
		}
		event void On_Tick()
		{
			if (m_last_open_state != Is_Open)
			{
				Log.Write("Open!");
				Update_State();

				if (Is_Open)
				{
					FX.Spawn_Untracked(Explode_Effect, this.Collision_Center, 0, this.Layer);
				}

				m_last_open_state = Is_Open;
			}
		}
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
		if (type == CollisionDamageType.Explosive)
		{
			Is_Open = true;
		}
		return true;
	}

	replicate(ReplicationPriority.High)
	{	
		Is_Open
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "actor_door_dungeon_up_destroyable_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
		CollisionComponent damage_sensor
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 16, 24);
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.Damage;
		}
		serialized(50) CollisionComponent collision
		{
			Shape 				= CollisionShape.Rectangle;
			Area 				= Vec4(0, 0, 64, 37);
			Type				= CollisionType.Solid;		
			Group				= CollisionGroup.Environment;
			Collides_With		= CollisionGroup.Player | CollisionGroup.Enemy;
			Blocks_Path_Spawns	= true;
		}
	}
}