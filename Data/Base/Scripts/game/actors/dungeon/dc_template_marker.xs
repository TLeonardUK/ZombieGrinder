// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.components.collision;
using system.actors.actor;
using system.scene;
using system.time;

// Must match native enum MapRandGenTemplateType.
public enum DC_Template_Type
{
	Unused_Door_Top = 0,
	Unused_Door_Bottom = 1,
	Unused_Door_Left = 2,
	Unused_Door_Right = 3,
}

[
	Placeable(true), 
	Name("DC Template Marker"), 
	Description("Defines a tilemap chunk used for various things when building random dungeon.") 
]
public class DC_Template_Marker : Actor
{	
	[ 
		Name("Template Type"),
		Description("Template type identifier, needs to match native enum MapRandGenTemplateType.")
	]
	public serialized(150) DC_Template_Type Template_Type = DC_Template_Type.Unused_Door_Top;
	
	DC_Template_Marker() 
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 0.0, 255.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
	}
}
