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
	Name("Bounding Area"), 
	Description("Toggleable solid collision.") 
]
public class Bounding_area : Actor
{	
	Bounding_area() 
	{
		Bounding_Box	= Vec4(0, 0, 64, 64);
		collision.Area 	= Bounding_Box;
	}
	
	default state Spawning
	{
		event void On_Enter()
		{
		}
		event void On_Editor_Property_Change()
		{
			collision.Area = Bounding_Box;
		}
	} 
	
	public override void On_Trigger()
	{
		collision.Enabled = !collision.Enabled;
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 128.0, 255.0, 128.0);
			Scale = true;
			Visible = false;
			Editor_Visible = true;
		}
		serialized(50) CollisionComponent collision
		{
			Shape 			= CollisionShape.Rectangle;
			Area 			= Vec4(0, 0, 64, 64);
			Type			= CollisionType.Solid;		
			Group			= CollisionGroup.Environment;
			Collides_With	= CollisionGroup.All;
			Velocity		= Vec2(0.0f, 0.0f);
			Blocks_Path_Spawns	= true;
		}
	}
}
