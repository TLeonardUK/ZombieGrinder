// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------

public enum CollisionShape
{
	Rectangle,
	Oval,
	Line,
	Circle
}

public enum CollisionType
{
	Non_Solid,
	Solid
}

public enum CollisionGroup
{
	NONE		= 0,
	Player		= 1,
	Enemy		= 2,
	Environment	= 4,
	Particle	= 8,
	Volume		= 16,
	Damage		= 32,
	All			= 255	// Should be 0x7FFFFFFF but hex seems to be fucked in the compiler atm. FIX
}  

public enum CollisionDamageType
{
	NONE		= 0,

	Projectile	= 1,
	Melee		= 2,
	Fire		= 3,
	Ice			= 4,
	Explosive	= 5,
	Acid		= 6,
	Shock		= 7,
	Poison		= 8,
    Laser       = 9,

	// Gross, these need to go somewhere else.
	Buff_Ammo	= 10,	//k	
	Buff_Health	= 11,	//k
	Buff_Damage	= 12,	//k
	Buff_Price	= 13,	//k
	Buff_Reload	= 14,	//k
	Buff_ROF	= 15,	//k	
	Buff_Speed	= 16,	//k	
	Buff_XP		= 17,	//k

	COUNT		= 18
}

[
	Name("CollisionComponent"), 
	Description("When attached to an actor it provides collision response.") 
]
public native("CollisionComponent") class CollisionComponent
{
	// Creates a new collision component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") CollisionComponent();	
	
	// Gets/sets the shape of the collision.
	property serialized(1) CollisionShape Shape
	{
		public native("Set_Shape") void Set(CollisionShape val);
		public native("Get_Shape") CollisionShape Get();
	}
	
	// Gets/sets the type of collision.
	property serialized(1) CollisionType Type
	{
		public native("Set_Type") void Set(CollisionType val);
		public native("Get_Type") CollisionType Get();
	}
	
	// Gets/sets our collision group.
	property serialized(1) CollisionGroup Group
	{
		public native("Set_Group") void Set(CollisionGroup val);
		public native("Get_Group") CollisionGroup Get();
	}
	
	// Gets/sets bitmask of groups we collide with.
	property serialized(1) CollisionGroup Collides_With
	{
		public native("Set_Collides_With") void Set(CollisionGroup val);
		public native("Get_Collides_With") CollisionGroup Get();
	}
	
	// Gets/sets the area of the collision.
	property serialized(1) Vec4 Area
	{
		public native("Set_Area") void Set(Vec4 val);
		public native("Get_Area") Vec4 Get();
	}

	// Gets/sets the enabled state of this collision component.
	property serialized(10) bool Enabled
	{
		public native("Set_Enabled") void Set(bool val);
		public native("Get_Enabled") bool Get();
	}
	
	// Gets/sets if this collision component blocks path generation.
	property serialized(90) bool Blocks_Paths
	{
		public native("Set_Blocks_Paths") void Set(bool val);
		public native("Get_Blocks_Paths") bool Get();
	}	
	
	// Gets/sets if this collision component blocks path based spawns
	property serialized(300) bool Blocks_Path_Spawns
	{
		public native("Set_Blocks_Path_Spawns") void Set(bool val);
		public native("Get_Blocks_Path_Spawns") bool Get();
	}	

	// Gets/sets the velocity imparted onto colliding objects.
	property serialized(100) Vec2 Velocity
	{
		public native("Set_Velocity") void Set(Vec2 val);
		public native("Get_Velocity") Vec2 Get();
	}

	// Gets/sets if this collision attempts to smooth out cornering, expensive, should only be enabled for PC's.
	property serialized(2600) bool Smoothed
	{
		public native("Set_Smoothed") void Set(bool val);
		public native("Get_Smoothed") bool Get();
	}	
    
	public native("Is_Colliding") bool Is_Colliding();
}