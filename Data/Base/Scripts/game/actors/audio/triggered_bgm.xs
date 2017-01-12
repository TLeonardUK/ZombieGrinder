// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;
using system.audio;

[
	Placeable(true), 
	Name("Triggered BGM"), 
	Description("When triggered, switches background music to the given file.") 
]
public class Triggered_BGM : Actor
{
	[ 
		EditType("Audio") 
	]
	public serialized(1) string Audio_Name = "";
	
	private serialized(40) bool Active = false;

	private bool Played = false;

	Triggered_BGM()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}
	
	public override void On_Trigger()
	{
		Active = !Active;
		if (Active == true)
		{
			Played = false;
		}
	}

	public event void On_Tick()
	{
		if (Active == true && Played == false)
		{
			Audio.PlayBGM(Audio_Name);
			Played = true;
		}
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