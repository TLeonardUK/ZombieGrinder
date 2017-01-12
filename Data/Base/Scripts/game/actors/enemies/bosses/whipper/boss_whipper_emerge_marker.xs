// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Whipper Boss Emerge Points"), 
	Description("Point of intrest for AI trying to go through the level. Should be continuously linked from start-to-end of the map.") 
]
public class Boss_Whipper_Emerge_Marker : Actor
{
	[ 
		Name("Is Initial"), 
		Description("If true the boss will emerge from this marker.") 
	]
	public serialized(1) bool Is_Initial = false;
	
	Boss_Whipper_Emerge_Marker()
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
			Frame 			= "actor_shape_rect_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
