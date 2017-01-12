// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Decal"), 
	Description("Used to place a static decoration in the level. As soon as its spawned it hibernates.") 
]
public class Decal : Actor
{
	Decal()
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
	
	// Invoked when triggers are linked to this actor and invoked.
	public override void On_Trigger()
	{
		sprite.Visible = !sprite.Visible;
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "decal_default_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
	}
}