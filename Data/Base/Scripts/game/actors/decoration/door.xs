// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using game.actors.interactable.interactable;

[
	Placeable(true), 
	Name("Door"), 
	Description("Can either be triggered to open/close, or be interacted with..") 
]
public class Door : Actor, Interactable
{
	[ 
		EditType("Animation"),
		Description("Animation to play when opening. Closing is the same in reverse.")
	]
	public serialized(1) string Open_Animation = "actor_door_laser_redh_open";
	
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
		Description("If true the player can interact with it to open/close it.")
	]
	public serialized(1) bool Can_Interact_Open = true;
		
	[ 
		Description("If true the player can close the door once open.")
	]
	public serialized(1) bool Can_Close = true;
	
	[ 
		Description("If true this door will open when pawns are in proximity to it, and close when they aren't.")
	]
	public serialized(101) bool Proximity_Opened = false;
	
	[ 
		Description("Radius to use for proximity opening.")
	]
	public serialized(101) float Proximity_Radius = 72.0f;

	[ 
		Description("How much it costs the user to open this door.")
	]
	public serialized(1) int Cost = 0;
	
	[ 
		Description("Fraction of total height to use for collision height")
	]
	public serialized(101) float Collision_Height_Fraction = 0.25;

	// How close enemies have to be for the door to auto-open.
	private float Enemy_Open_Radius = 26.0f;

	private int m_last_open_state = -1;

	Door()
	{
		Bounding_Box	= Vec4(0, 0, 64, 37);
		collision.Area 	= Vec4(0, 0, 64, 37);
		Store_Initial_State();
	}

	private void Store_Initial_State()
	{		
		Vec4 bb = this.Bounding_Box;

		// Horizontal
		if (bb.Width * 1.5f > bb.Height)
		{
			float height = bb.Height * Collision_Height_Fraction;
			collision.Area = Vec4(bb.X, bb.Y + (bb.Height - height), bb.Width, height);
		}
		// Vertical
		else
		{
			collision.Area = bb;
		}

		//m_last_open_state = <int>Is_Open;

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
		
		sprite.Animation = Open_Animation;
		sprite.Animation_Speed =  Is_Open ? 1.0f : -1.0f;
		sprite.Frame = "";
		sprite.Depth_Bias = collision.Area.Y + (collision.Area.Height * 0.5f);
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
			if (Network.Is_Server())
			{
				if (Proximity_Opened)
				{
					float distance = Scene.Distance_To_Closest_Actor_Of_Type(this.Collision_Center, typeof(Pawn));		 
					if (Is_Open)
					{
						if (distance > Proximity_Radius * 1.5f)
						{
							Is_Open = false;
						}
					}
					else
					{
						if (distance < Proximity_Radius)
						{
							Is_Open = true;
						}
					}
				}
				else if (Can_Interact_Open && Can_Close)
				{
					float distance = Scene.Distance_To_Closest_Actor_Of_Type(this.Collision_Center, typeof(Enemy));		 
					if (!Is_Open)
					{
						if (distance < Enemy_Open_Radius)
						{
							Is_Open = true;
						}
					}
				}
			}

			if (m_last_open_state != <int>Is_Open)
			{
				Audio.Play3D(Open_Sound, this.Center);
				Update_State();
				m_last_open_state = <int>Is_Open;
			}
		}
	}

	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return Cost;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		if (Is_Open)
		{
			return Locale.Get("#buy_close_prompt");
		}
		else
		{
			return Locale.Get("#buy_open_prompt");
		}
	}
	
	public override bool Can_Interact(Pawn other)
	{
		if (Proximity_Opened)
		{
			return false;
		}

		// Something in the way of the closed door colllision?
		if (Is_Open && collision.Is_Colliding())
		{
			return false;
		}

		return Can_Interact_Open && (!Is_Open || Can_Close);
	}

	public override void Interact(Pawn other)
	{		
		rpc(RPCTarget.Server, RPCTarget.None) RPC_Toggle();
	}

	public rpc void RPC_Toggle()
	{
		Is_Open = !Is_Open;
	}

	private override void On_Trigger()
	{
		Is_Open = !Is_Open;
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
			Frame = "actor_door_laser_redh_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
			Object_Mask = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		serialized(50) CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 64, 37);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Environment;
			Collides_With	= CollisionGroup.Player | CollisionGroup.Enemy;
			Blocks_Path_Spawns	= true;
		}
	}
}