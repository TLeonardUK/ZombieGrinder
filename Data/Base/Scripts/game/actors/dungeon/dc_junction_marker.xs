// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

[
	Placeable(true), 
	Name("DC Junction Marker"), 
	Description("Defines a junction that is used by the dungeon-crawler rand-gen to link rooms together.") 
]
public class DC_Junction_Marker : Actor
{	
	DC_Junction_Marker() 
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 128.0, 0.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
	}
}
