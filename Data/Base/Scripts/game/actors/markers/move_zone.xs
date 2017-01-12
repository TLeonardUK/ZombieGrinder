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
	Name("Move Zone"), 
	Description("User to effect movement of collision objects.") 
]
public class Move_Zone : Actor
{	
	Move_Zone() 
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
			Type			= CollisionType.Non_Solid;		
			Group			= CollisionGroup.Volume;
			Collides_With	= CollisionGroup.All;
			Velocity		= Vec2(0.0f, 1.0f);
		}
	}
}
