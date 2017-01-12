// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Player Spawn Cutscene Point"), 
	Description("If the player spawns from a spawn point with a cutscene start tag refering to this, they will walk a path from here to the spawn point before handing control to the player.") 
]
public class Player_Spawn_Cutscene_Point : Actor
{
	Player_Spawn_Cutscene_Point()
	{
		Bounding_Box = Vec4(0, 0, 64, 64);
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
			Frame = "actor_player_start_cutscene_point_default_0";
			Color = Vec4(128.0, 128.0, 128.0, 255.0);
			Visible = false;
			Editor_Visible = true;
		}
	}
}
