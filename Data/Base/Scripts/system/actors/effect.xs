// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.components.effect;
using system.components.sprite;
using runtime.log;

[
	Placeable(true), 
	Name("Effect"), 
	Description("Spawned particle effect.") 
]
public class Effect : Actor
{
	private Actor m_attached_to = null;
	private Vec3 m_attach_offset;

	public bool DisposeOnFinish = false;

	Effect()
	{
		Bounding_Box = Vec4(0, 0, 16, 16);
	}

	void Attach_To(Actor attach)
	{
		m_attached_to = attach;
	}
	
	void Attach_To_Offset(Actor attach, Vec3 offset)
	{
		m_attached_to = attach;
		m_attach_offset = offset;
	}

	EffectComponent Get_Component()
	{
		return effect;
	}

	event void On_Tick()
	{
		bool bDispose = false;

		if (m_attached_to != null)
		{
			if (Scene.Is_Spawned(m_attached_to))
			{
				this.Position = Vec3(m_attached_to.World_Bounding_Box.Center(), 0.0f) + m_attach_offset;
			}
			else
			{
				bDispose = true;
			}
		}

		if (bDispose || (DisposeOnFinish && effect.Is_Finished))
		{
			m_attached_to = null;
			Scene.Despawn(this);
			return;
		}

		if (m_attached_to == null && !DisposeOnFinish)
		{
			Log.Write("Hibernating effect that dosen't require tick.");
			this.Hibernate();
		}
	}
	
	public override void On_Trigger()
	{
		effect.Paused = false;
		effect.Restart();

		this.Wake_Up();
	}

	components
	{
		serialized(1) EffectComponent effect
		{
			Effect_Name = "player_spawn";
			Visible = true;
			Paused = false;
		}
		SpriteComponent sprite
		{
			Frame 			= "actor_emitter_default_0";
			Color 			= Vec4(255.0, 255.0, 255.0, 255.0);
			Visible 		= false; 
			Editor_Visible 	= true;
		}
	}
}
