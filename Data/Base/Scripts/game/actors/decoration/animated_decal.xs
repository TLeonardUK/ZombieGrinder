// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Animated Decal"), 
	Description("Used to place an animated decoration in the level. Animation starts when decal is triggered.") 
]
public class Animated_Decal : Actor
{
	// Gets/sets the sprite animation.
	[ 
		EditType("Animation") 
	]
	public serialized(1) string Animation_Name = "";

	Animated_Decal()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	
	public override void On_Trigger()
	{
		sprite.Visible = !sprite.Visible;
		sprite.Animation = Animation_Name;
		sprite.Frame = "";
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "decal_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
	}
}