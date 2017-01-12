// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.actors.interactable.interactable;

[
	Placeable(true), 
	Name("Boss Whipper Door"), 
	Description("Dungeon doors support locking/sealing/etc. They should be placed in all levels and set to open by default.") 
]
public class Boss_Whipper_Door : Actor
{
	[ 
		EditType("Animation"),
		Description("Animation to play when opening. Closing is the same in reverse.")
	]
	public serialized(1) string Open_Animation = "actor_door_forest_dungeon_up_open";
	
	[ 
		EditType("Audio"),
		Description("Sound to play when opening or closing door.")
	]
	public serialized(1) string Open_Sound = "sfx_objects_doors_place";
	
	[ 
		Description("True if this door is currently open or not.")
	]
	public serialized(1) bool Is_Open = false;

	[ 
		Description("True if this door is the entrance.")
	]
	public serialized(1) bool Is_Entrance = false;
		
    private int m_last_open_state = -1;

    private float m_last_speed = 0.0f;

	Boss_Whipper_Door()
	{
		Bounding_Box	= Vec4(0, 0, 48, 48);
		collision.Area 	= Vec4(0, 0, 48, 48);
		Update_State();
	}

	private void Update_State()
	{
		bool bOpen = Is_Open;

		if (bOpen)
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
		
		//	collision.Area = Vec4(16.0f, 16.0f, 16.0f, 16.0f);
		if (Open_Animation.Contains("_up_") || Open_Animation.Contains("_down_"))
		{
			collision.Area = Vec4(0.0f, 16.0f, 48.0f, 16.0f);
		}
		else
		{
			collision.Area = Vec4(16.0f, 0.0f, 16.0f, 48.0f);		
		}

        float speed = 1.0f;
        if (!bOpen)
        {
            speed = -1.0f;
        }
		
		sprite.Animation = Open_Animation;
	    sprite.Animation_Speed = speed;
		sprite.Frame = "";
		sprite.Depth_Bias = this.Bounding_Box.Height;//collision.Area.Y + (collision.Area.Height * 0.5f);
         
        m_last_speed = speed;

		Log.Write("Playing animation (Is_Open="+Is_Open+") "+sprite.Animation+" with speed "+sprite.Animation_Speed);
	}

	default state Idle
	{
		event void On_Enter()
		{
			Update_State();
		}
		event void On_Editor_Property_Change()
		{
			Update_State();
		}
		event void On_Tick()
		{
			if (m_last_open_state != <int>Is_Open)
			{
				Audio.Play3D(Open_Sound, this.Center);
				Update_State();
				m_last_open_state = <int>Is_Open;
			}
    	}
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
			Frame = "actor_door_forest_dungeon_up_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
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