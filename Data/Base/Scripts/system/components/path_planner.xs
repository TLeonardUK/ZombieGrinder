// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.collision;

[
	Name("Path Planner Component"), 
	Description("Used to latently generate paths between 2 points and produce movement vectors to move between them..") 
]
public native("PathPlannerComponent") class PathPlannerComponent
{ 
	// Creates a new sprite component and binds it to the executing actor.
	// Shouldn't be called directly, use a Components {} block.
	public native("Create") PathPlannerComponent();
	
	// Gets if a path is available.
	property bool Has_Path
	{
		public native("Get_Has_Path") bool Get();
	}

	// Gets/sets the target position to move towards.
	property Vec3 Target_Position
	{
		public native("Set_Target_Position") void Set(Vec3 value);
		public native("Get_Target_Position") Vec3 Get();
	}

	// Gets/sets the source position to move from.
	property Vec3 Source_Position
	{
		public native("Set_Source_Position") void Set(Vec3 value);
		public native("Get_Source_Position") Vec3 Get();
	}

	// Gets/sets how far the target position has to move before the 
	// path will be automatically regenerated.
	property float Regenerate_Delta
	{
		public native("Set_Regenerate_Delta") void Set(float value);
		public native("Get_Regenerate_Delta") float Get();
	}
	
	// Gets/sets the collision group used to determine what path nodes we can walk through.
	property CollisionGroup Collision_Group
	{
		public native("Set_Collision_Group") void Set(CollisionGroup value);
		public native("Get_Collision_Group") CollisionGroup Get();
	}

	// Gets the movement vector we need to move to get towards the next path node.
	property Vec3 Movement_Vector
	{
		public native("Get_Movement_Vector") Vec3 Get();
	}

	// Gets/sets if this works on the client side - default is server only.
	property bool Client_Side
	{
		public native("Set_Client_Side") void Set(bool value);
		public native("Get_Client_Side") bool Get();
	}
	
	// Gets if the target is inside our LOS.
	property bool Target_In_LOS
	{
		public native("Get_Target_In_LOS") bool Get();
	}
	
	// Gets if we are at our target.
	property bool At_Target
	{
		public native("Get_At_Target") bool Get();
	}
}