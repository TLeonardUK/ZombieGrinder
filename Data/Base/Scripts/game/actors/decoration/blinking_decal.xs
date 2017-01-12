// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Blinking Decal"), 
	Description("Blinks every so often and plays animation/sound.") 
]
public class Blinking_Decal : Actor
{
	[ 
		EditType("Animation") 
	]
	public serialized(1) string Blink_Animation = "actor_explosive_tick";
	
	[ 
		EditType("Audio") 
	]
	public serialized(1) string Blink_Sound = "sfx_objects_doors_countdown";
	
	public serialized(1) float Blink_Interval = 1.0f;

	private float m_blink_timer = 0.0f;

	Blinking_Decal()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	public event void On_Tick()
	{
		if (sprite.Visible)
		{
			m_blink_timer -= Time.Get_Delta_Seconds();
			if (m_blink_timer <= 0.0f)
			{
				if (Blink_Animation != "")
				{
					sprite.Play_Oneshot_Animation(Blink_Animation, 1.0f);
				}
				if (Blink_Sound != "")
				{
					Audio.Play3D(Blink_Sound, this.Position);
				}
				m_blink_timer = Blink_Interval;
			}
		}
	}
	
	public override void On_Trigger()
	{
		sprite.Visible = !sprite.Visible;
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Frame = "actor_explosive_0";
			Color = Vec4(255.0, 255.0, 255.0, 255.0);
		}
	}
}