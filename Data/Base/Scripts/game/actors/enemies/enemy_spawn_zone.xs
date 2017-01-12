// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Enemy Spawn Zone"), 
	Description("Zone from which enemies can spawn during wave modes.") 
]
public class Enemy_Spawn_Zone : Actor
{
	[ 
		Name("Must By Out Of Sight"), 
		Description("If set this spawn zone must be off screen to be used.") 
	]
	public serialized(1) bool Must_Be_Out_Of_Sight = true;
	
	[ 
		Name("Preferrential Distance"), 
		Description("If player is within this distance of this spawn zone it will be added to the priority list for spawning.") 
	]
	public serialized(1) float Preferrential_Distance = 256.0f;

	[ 
		Name("Maximum Spawns"), 
		Description("Maximum number of spawns from this zone, or zero for unlimited.") 
	]
	public serialized(155) int Max_Spawns = 0;
	public int Spawns = 0;
	
	[ 
		Name("Throttles Spawns"), 
		Description("If set then spawns from this zone will cause spawn timer throttling.") 
	]
	public serialized(155) bool Throttles_Spawns = true;

	Enemy_Spawn_Zone()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	
	public override void On_Trigger()
	{
		Enabled = !Enabled;
		Spawns = 0;
	}

	components
	{
		SpriteComponent sprite
		{
			Frame 			= "actor_zombie_zone_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
