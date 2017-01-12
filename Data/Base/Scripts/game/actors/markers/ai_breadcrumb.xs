// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("AI Breadcrumb"), 
	Description("Point of intrest for AI trying to go through the level. Should be continuously linked from start-to-end of the map.") 
]
public class AI_Breadcrumb : Actor
{
	[ 
		Name("Interact"), 
		Description("If set to true, the AI will attempt to interact when it gets to this breadcrumb.") 
	]
	public serialized(1) bool bInteractAtEnd = false;
	
	[ 
		Name("Wait For"), 
		Description("If set to true, the AI will wait for the given amount of time at this breadcrumb before moving on.") 
	]
	public serialized(1) float bWaitFor = 0.0f;
	
	AI_Breadcrumb()
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
			Frame 			= "decal_ai_breadcrumb_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
