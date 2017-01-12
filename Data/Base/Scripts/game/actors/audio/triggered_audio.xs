// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;

[
	Placeable(true), 
	Name("Triggered Audio"), 
	Description("When triggered, plays an audio clip.") 
]
public class Triggered_Audio : Actor
{
	[ 
		EditType("Audio") 
	]
	public serialized(1) string Audio_Name = "";

	Triggered_Audio()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	
	public override void On_Trigger()
	{
		Audio.Play3D(Audio_Name, Position);
	}

	components
	{
		SpriteComponent sprite
		{
			Frame			= "actor_sound_source_default_0";
			Color			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}