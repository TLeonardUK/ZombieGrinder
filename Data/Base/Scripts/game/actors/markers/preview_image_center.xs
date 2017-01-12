// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Preview Image Center"), 
	Description("Center of the preview-image used when publishing the map / display in menus.") 
]
public class Preview_Image_Center : Actor
{
	Preview_Image_Center()
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
			Frame 			= "actor_preview_center_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
