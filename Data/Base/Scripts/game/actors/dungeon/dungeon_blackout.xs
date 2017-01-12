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
	Name("Dungeon Blackout"), 
	Description("Programmatically placed blackout blocks. They fade in when the player gets close, and fades out when they get further away.") 
]
public class Dungeon_Blackout : Actor
{	
	Dungeon_Blackout() 
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	default state Idle
	{
		event void On_Tick()
		{
			Dungeon_Crawler_Mode mode = <Dungeon_Crawler_Mode>Scene.Active_Game_Mode;
			if (mode != null && !mode.IsDark)
			{
				Scene.Despawn(this);
				return;
			}
			
			float target_alpha = 255.0f;
			Vec4 bbox = this.World_Bounding_Box + Vec4(-8.0f, -8.0f, 16.0f, 16.0f);

			this.Tick_Area = bbox;

			if (Scene.Are_Actors_Colliding(typeof(Human_Player), bbox))
			{
				target_alpha = 0.0f;
			}

			sprite.Color = Vec4(0.0f, 0.0f, 0.0f, Math.Lerp(sprite.Color.W, target_alpha, 7.0f * Time.Get_Delta_Seconds()));
		}
	}
		
	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_shape_rect_0";
			Color = Vec4(0.0, 0.0, 0.0, 255.0f);
			Scale = true;
			Visible = true;
			Editor_Visible = false;
		}
	}
}
