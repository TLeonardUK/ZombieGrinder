// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Player Spawn"), 
	Description("Defines where a player can spawn.") 
]
public class Player_Spawn : Actor
{
	[ 
		Name("Intro Point Link"), 
		Description("If set the player will move from this intro point to this spawn point on spawn.") 
	]
	public serialized(3500) string Intro_Point_Link;

	[ 
		Name("Team Index"), 
		Description("Index of team that spawns on this actor. Set to -1 to allow all teams to spawn.") 
	]
	public serialized(1) int Team_Index;

	[ 
		Name("Active"), 
		Description("If set this spawn is active and can be used for spawning.") 
	]
	public serialized(30) int Active = true;

	[ 
		Name("Spawn Direction"), 
		Description("Direction player should face when spawned.") 
	] 
	public serialized(1) Direction Spawn_Direction;
	
	[ 
		Name("Spawn Order"), 
		Description("Order in which spawns should be used.") 
	] 
	public serialized(3501) int Spawn_Order;
	
	Player_Spawn()
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
	
	private override void On_Trigger()
	{
		Active = !Active;
	}

	components
	{
		SpriteComponent sprite
		{
			Frame = "actor_player_start_default_0";
			Color = Vec4(128.0, 128.0, 128.0, 255.0);
			Visible = false;
			Editor_Visible = true;
		}
	}
}
