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
	Name("Capture Zone"), 
	Description("Used in CTB game modes to trigger a bag capture.") 
]
public class Capture_Zone : Actor
{	
	[ 
		Name("Team Index"), 
		Description("Team index of pawns that can trigger this. -1 = all teams.") 
	]
	public serialized(1) int Team_Index = 0;
	
	Capture_Zone() 
	{
		Bounding_Box	= Vec4(0, 0, 64, 64);
		collision.Area 	= Bounding_Box;
	}
	
	default state Spawning
	{
		event void On_Tick()
		{
			//Hibernate();
		}
		event void On_Editor_Property_Change()
		{
			collision.Area 	= Bounding_Box;
		}
		event void On_Touch(CollisionComponent component, Actor other)
		{
			if (other != null)
			{
				if (Team_Index == -1 || (other.Owner != null && other.Owner.Team_Index == Team_Index)) // Owned by a player.
				{
					CTB_Game_Mode gm = <CTB_Game_Mode>Scene.Active_Game_Mode;
					if (gm != null)
					{
						gm.Trigger_Capture(other);
					}
				}
			}
		}
	} 
	
	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 255.0, 0.0, 128.0);
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
			Collides_With	= CollisionGroup.Player;
		}
	}
}
