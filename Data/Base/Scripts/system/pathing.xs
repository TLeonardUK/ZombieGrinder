// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.effect;

public struct Path_Spawn_Point
{
	bool Is_Valid;
	Vec4 Area;
}

[
	Placeable(false), 
	Name("Pathing"), 
	Description("Global class, responsible for path generation.") 
]
public native("Pathing") class Pathing
{
	// Gets the pylon that built the path at the given position.
	public static native("Get_Pylon_By_Position") Actor Get_Pylon_By_Position(Vec3 position);

	// Finds a given position on the path node grid where the given bounding_box can be placed that is;
	//		- Not encroached by other objects (based on bounding box).
	//		- Whos center is inside the maximum radius (based on local camera center).
	//		- Whos center is outside the minimum radius (based on local camera center).
	//		- Is not visible on any clients screens.
	public static native("Find_Random_Spawn_Area") Path_Spawn_Point Find_Random_Spawn_Area(Vec4 bounding_box, float min_radius, float max_radius, bool can_spawn_on_screen);
}
