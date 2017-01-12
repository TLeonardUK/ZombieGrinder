// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Path_Pylon"), 
	Description("Path generation expands from these pylons. Place one in each zone you want to be pathable.") 
]
public class Path_Pylon : Actor
{
	Path_Pylon()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Enter()
		{
			Hibernate();
		}
	}

	components
	{
		SpriteComponent sprite
		{
			Frame 			= "decal_inside_insidemapmarker_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
