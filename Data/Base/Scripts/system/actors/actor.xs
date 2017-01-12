 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.network;
using runtime.math;

public enum Direction
{
	S,
	SE,
	E,
	NE,
	N,
	NW,
	W,
	SW,

	COUNT
}

public enum Tick_Priority_Type
{
	Early	= -1,
	Normal	=  0,
	Late	=  1,
	Final	=  2,
}

public static class Direction_Helper
{
	public static string[] Mnemonics = 
		{
			"s",
			"se",
			"e",
			"ne",
			"n",
			"nw",
			"w",
			"sw"	
		};
		
	public static float[] Angles =
		{
			0.0,								//S
			-Math.QuarterPI,					//SE
			-Math.HalfPI,						//E
			-(Math.HalfPI + Math.QuarterPI),	//NE
			Math.PI,							//N
			(Math.HalfPI + Math.QuarterPI),		//NW
			Math.HalfPI,						//W
			Math.QuarterPI						//SW
		};

	public static Vec3[] Vectors =
		{
			new Vec3(-Math.Sin(Angles[0]), Math.Cos(Angles[0]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[1]), Math.Cos(Angles[1]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[2]), Math.Cos(Angles[2]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[3]), Math.Cos(Angles[3]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[4]), Math.Cos(Angles[4]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[5]), Math.Cos(Angles[5]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[6]), Math.Cos(Angles[6]), 0).Unit(),
			new Vec3(-Math.Sin(Angles[7]), Math.Cos(Angles[7]), 0).Unit()
		};

	public static Direction[] Direction4 =
		{
			Direction.N,
			Direction.S,
			Direction.E,
			Direction.W
		};
	
	public static Direction[] Direction8 =
		{
			Direction.S,
			Direction.SE,
			Direction.E,
			Direction.NE,
			Direction.N,
			Direction.NW,
			Direction.W,
			Direction.SW
		};

	public static Direction Rand4Dir()
	{
		return Direction4[Math.Rand(0, 4)];
	}

	public static Direction Rand8Dir()
	{
		return Direction8[Math.Rand(0, 8)];
	}

	public static Direction Calculate_Direction(float x, float y, Direction dir, bool limit_to_4_dir = false)
	{
		// Ignore very slightly diagonal movements.
		float abs_x = Math.Abs(x);
		float abs_y = Math.Abs(y);

		float epsilon = 0.0001f;

		if (abs_x < abs_y * 0.5f) 
		{
			x = 0.0f;
		}
		if (abs_y < abs_x * 0.5f) 
		{
			y = 0.0f;
		}

		if (limit_to_4_dir == true)
		{		
			if (x < 0.0f && y != 0.0f)
			{
				if (dir == Direction.E) 
				{
					dir = Direction.W;
				}
				if (y < 0.0f && dir == Direction.S)
				{
					dir = Direction.N;
				}
				if (y > 0.0f && dir == Direction.N)
				{
					dir = Direction.S;
				}
			}
			else if (x > 0.0f && y != 0.0f)
			{
				if (dir == Direction.W) 
				{
					dir = Direction.E;
				}
				if (y < 0.0f && dir == Direction.S)
				{
					dir = Direction.N;
				}
				if (y > 0.0f && dir == Direction.N)
				{
					dir = Direction.S;
				}
			}
			else if (x < 0.0f)
			{
				return Direction.W;
			}
			else if (x > 0.0f)
			{
				return Direction.E;
			}
			else if (y < 0.0f)
			{
				return Direction.N;
			}
			else if (y > 0.0f)
			{
				return Direction.S;
			}
			
			return dir;
		}
		else
		{
			switch (true)
			{
				case (x > epsilon  && y > epsilon):		{ dir = Direction.SE; break; }
				case (x > epsilon  && y < -epsilon):	{ dir = Direction.NE; break; }
				case (x < -epsilon && y > epsilon):		{ dir = Direction.SW; break; }
				case (x < -epsilon && y < -epsilon):	{ dir = Direction.NW; break; }
				case (x < 0):							{ dir = Direction.W; break; }
				case (x > 0):							{ dir = Direction.E; break; }
				case (y > 0):							{ dir = Direction.S; break; }
				case (y < 0):							{ dir = Direction.N; break; }
			}
		}

		return dir;
	}
	
}

[
	Name("Actor"), 
	Description("Base class for all actors.") 
]
public native("Actor") class Actor
{
	// Gets/Sets the tag of the actor, used for linking triggers together.
	property serialized(4) string Tag
	{
		public native("Set_Tag") void Set(string pos);
		public native("Get_Tag") string Get();
	}

	// Gets/Sets the link of the actor, used for linking triggers together.
	property serialized(4) string Link
	{
		public native("Set_Link") void Set(string pos);
		public native("Get_Link") string Get();
	}

	// Gets/Sets the enabled state.
	property serialized(1) bool Enabled
	{
		public native("Set_Enabled") void Set(bool pos);
		public native("Get_Enabled") bool Get();
	}
	
	// Gets/Sets the activated state. Use depends on actor.
	property bool Activated_By_Player
	{
		public native("Set_Activated_By_Player") void Set(bool pos);
		public native("Get_Activated_By_Player") bool Get();
	}
	
	// Gets/Sets the current position.
	property serialized(1) Vec3 Position
	{
		public native("Set_Position") void Set(Vec3 pos);
		public native("Get_Position") Vec3 Get();
	}
		
	// Gets/Sets the current rotation.
	property serialized(1) float Rotation
	{
		public native("Set_Rotation") void Set(float pos);
		public native("Get_Rotation") float Get();
	}
	
	// Gets/Sets the current scale.
	property serialized(1) Vec2 Scale
	{
		public native("Set_Scale") void Set(Vec2 pos);
		public native("Get_Scale") Vec2 Get();
	}
	
	// Gets/Sets the current layer.
	property serialized(1) int Layer
	{
		public native("Set_Layer") void Set(int pos);
		public native("Get_Layer") int Get();
	}
	
	// Gets/Sets the current depth bias.
	property serialized(1) float Depth_Bias
	{
		public native("Set_Depth_Bias") void  Set(float pos);
		public native("Get_Depth_Bias") float Get();
	}

	// Gets/Sets the Y offset relative to its bounding box used
	// for calculating depth.
	property serialized(6) float Depth_Y_Offset
	{
		public native("Set_Depth_Y_Offset") void  Set(float pos);
		public native("Get_Depth_Y_Offset") float Get();
	}
	
	// Gets/Sets the current bounding box.
	property serialized(1) Vec4 Bounding_Box
	{
		public native("Set_Bounding_Box") void Set(Vec4 pos);
		public native("Get_Bounding_Box") Vec4 Get();
	}
	
	// Gets/Sets the current world bounding box.
	property Vec4 World_Bounding_Box
	{
		public native("Get_World_Bounding_Box") Vec4 Get();
	}
		
	// Gets the center of the actor.
	property Vec3 Center
	{
		public native("Get_Center") Vec3 Get();
	}
	
	// Gets the center of this actors first collision component, if no collision component
	// is available then this will act the same as Center.
	property Vec3 Collision_Center
	{
		public native("Get_Collision_Center") Vec3 Get();
	}
	
	// Gets the bbox of this actors first collision component, if no collision component
	// is available then this will act the same as Center.
	property Vec4 Collision_Box
	{
		public native("Get_Collision_Box") Vec4 Get();
	}
	
	// Gets the bbox of this actors first collision component, if no collision component
	// is available then this will act the same as Center.
	property Vec4 World_Collision_Box
	{
		public native("Get_World_Collision_Box") Vec4 Get();
	}

	// If network synced this will return its current replication ID.
	property int Net_ID
	{
		public native("Get_Net_ID") int Get();
	}
	
	// Gets unsynced unique-id. Only valid for non-networked static objects.
	property int Unique_ID
	{
		public native("Get_Unique_ID") int Get();
	}
	
	// Gets/Sets the current owner.
	property NetUser Owner
	{
		public native("Get_Owner") NetUser Get();
		public native("Set_Owner") void Set(NetUser user);
	}
	
	// Gets/Sets the current parent actor. When the parent is 
	// destroyed all children will be as well.
	property Actor Parent
	{
		public native("Get_Parent") Actor Get();
		public native("Set_Parent") void  Set(Actor parent);
	}
	
	// ---------------------------------------------------------------------------------------------------------
	// The below tick states should not normally be messed with. The only advantage to adjusting these
	// are if you have a lot of an actor in a map and you don't want to waste time ticking all of them.
	// Abusing them can result in wierd effects, you have been warned :)
	// ---------------------------------------------------------------------------------------------------------
	property bool Tick_Offscreen
	{
		public native("Get_Tick_Offscreen") bool Get();
		public native("Set_Tick_Offscreen") void Set(bool value);
	}	

	property Tick_Priority_Type Tick_Priority
	{
		public native("Get_Tick_Priority") Tick_Priority_Type Get();
		public native("Set_Tick_Priority") void Set(Tick_Priority_Type value);
	}	
		
	property Vec4 Tick_Area
	{
		public native("Get_Tick_Area") Vec4 Get();
		public native("Set_Tick_Area") void Set(Vec4 value);
	}	

	// Gets/Sets the radius from players that this actor should tick.
	// if this is set to 0 (default), the actor will always tick.


	// Gets/Sets the interval between ticks. 1=tick every frame, 2=tick every other frame, 3=tick every 3rd frame, etc.


	// Gets/Sets the tick priority. Actors to be ticked are sorted according to their priorties before ticking occurs.
	/*
		Tick_Radius		= 256.0f;
		Tick_Interval	= 2.0f;
		Tick_Priority	= ActorTickPriority.Normal;
		Tick_Offscreen	= false;
	*/

	// Moves to a given position with collision detection. Deferred to next frame.
	public native("MoveTo") void MoveTo(Vec3 pos);
	// Moves to a given position with no collision checks. All calls to MoveTo will be ignored on this frame.
	public native("Teleport") void Teleport(Vec3 pos);
	// Limits movement this frame to the given area.
	public native("LimitMovement") void LimitMovement(Vec4 area);
	
	// When call the actor hibernates and its code is no longer executed. 
	public native("Hibernate") void Hibernate();	
	// Wakes up the actor if its hibernating.
	public native("Wake_Up") void Wake_Up();		
	// Returns true if the actor is hibernating.
	public native("Is_Hibernating") bool Is_Hibernating();

	// Deactivates all components attached to this actor.
	public native("Deactivate_Components") void Deactivate_Components();

	// Returns how far we moved in the last frame.
	public native("Get_Last_Frame_Movement") Vec3 Get_Last_Frame_Movement();

	// Invoked when triggers are linked to this actor and invoked.
	public virtual void On_Trigger()
	{
	}

	// Returns the current direction for direction based actors. This is pretty hacky, its 
	// mainly here so I don't have to bother deriving Turrets from Pawns lol.
	public virtual Direction Get_Direction()
	{
		return Direction.N;
	}

	// Gets the team this actor belongs on.
	public virtual int Get_Team_Index()
	{
		return 0;
	}

	// Gets the NetUser this actor is imposing as - used for turrets linking to correct player etc.
	public virtual NetUser Get_Imposing_As()
	{
		return null;
	}
    
    public void Change_Owner(NetUser owner)
    {
        rpc(RPCTarget.All, RPCTarget.None) RPC_Change_Owner(owner == null ? -1 : owner.Net_ID);
    }

    public rpc void RPC_Change_Owner(int net_owner_id)
    {
        NetUser user = Network.Get_User(net_owner_id);
        if (user == null) 
        {
            return;
        }
        this.Owner = user;
    }
}