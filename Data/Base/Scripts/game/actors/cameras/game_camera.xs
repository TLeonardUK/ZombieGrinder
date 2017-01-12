// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.camera;
using system.network;
using system.display;
using system.time;
using runtime.math;
using game.effects.chromatic_arboration_post_process_fx; 

[
	Placeable(false), 
	Name("Game Camera"), 
	Description("Base functionality for game cameras.") 
]
public class Game_Camera : Camera
{
	protected bool m_shaking;
	protected float m_shake_time;
	protected float m_shake_intensity;
	protected float m_shake_current_time;

	protected bool m_viewport_max;

	const float Shake_Threshold = 0.01f;
	const float Shake_Falloff_Distance_Min	= 64.0f;
	const float Shake_Falloff_Distance_Max	= 600.0f;
	const float Shake_Lerp_Factor			= 75.0f;
	
	const float Instant_Lerp_Distance		= 120.0;
	const float Max_Viewport_Zoom_Radius	= 140.0;
	const float Viewport_Buffer				= 64.0;

    const float Viewport_Idle_Distance      = 0.9f;

	protected Vec4 Calculate_Ideal_Viewport(Actor[] framing)
	{
		Vec2 res = Scene.Get_Camera_Screen_Resolution(this);
		Vec4 scene_bounds = Scene.Get_Bounds();
		Vec4 screen_viewport = Scene.Get_Camera_Screen_Viewport(this);
		float aspect = screen_viewport.Width / screen_viewport.Height;
		float inv_aspect = (1.0 / aspect);
		
		Vec2 center = Vec2(0,0);  

		// Calculate centroid of viewport.
		foreach (Actor actor in framing)
		{
			center = center + actor.World_Bounding_Box.Center();		
		}		  
		center = center / framing.Length();
	
		// Calculate maximum radius.
		float max_radius = 0.0;
		foreach (Actor actor in framing)
		{ 
			Vec2 actor_center = actor.World_Bounding_Box.Center();
			Vec2 diff = (actor_center - center);
			float diff_x = Math.Abs(diff.X) * inv_aspect;
			float diff_y = Math.Abs(diff.Y);
			
			if (diff_x > max_radius)
			{
				max_radius = diff_x;
			}			
			if (diff_y > max_radius)
			{
				max_radius = diff_y;
			}
		}	
 
		// Blow up radius a bit so we have a buffer around the edge of the screen.
		max_radius = Math.Min(Max_Viewport_Zoom_Radius, max_radius + Viewport_Buffer);
		m_viewport_max = (max_radius > Max_Viewport_Zoom_Radius * Viewport_Idle_Distance);
           
		// Figure out viewport size.
		float view_height = Math.Max(res.Y, max_radius * 2);
		float view_width = view_height * aspect;

		// Cap to scene bounds.
		if (view_width > scene_bounds.Width)
		{
			float diff = view_width - scene_bounds.Width;
			if (diff > 0 && view_width - diff > res.X)
			{
				view_width -= diff;
				view_height -= diff * inv_aspect;
				m_viewport_max = true;
			}
		}
		if (view_height > scene_bounds.Height)
		{
			float diff = view_height - scene_bounds.Height;
			if (diff > 0 && view_height - diff > res.Y)
			{
				view_height -= diff;
				view_width -= diff * aspect;
				m_viewport_max = true;
			}		 
		}
		
		// Return resultant viewport.
		return Vec4
		( 
			center.X - (view_width * 0.5),
			center.Y - (view_height * 0.5),
			view_width,
			view_height 
		);
	}
	
	protected Vec4 Lerp_Viewport(Vec4 from, Vec4 to, float delta)
	{
		Vec4 lerped;
		
		if ((from.Center() - to.Center()).Length() > Instant_Lerp_Distance)
		{
			lerped = to;
		}
		else
		{
			lerped = Math.Lerp(from, to, delta);
		}
	
		return lerped.Constrain(Scene.Get_Bounds());		
	}
	
	public override void Shake(float duration, float intensity, Vec3 position, Actor instigator, float chromatic_abberation_intensity)
	{
		Vec2 center = Get_Viewport().Center();
		float distance = (position.XY - center).Length();

		if (duration <= 0.0f || intensity <= 0.0f)
		{
			return;
		}
		
		if (!Options.Get_Bool("screen_shake_enabled"))
		{
			return;
		}

		// Scale effect based on distance to cause.
		float effect_delta = 1.0f;
		if (distance > Shake_Falloff_Distance_Min)
		{
			float range = Shake_Falloff_Distance_Max - Shake_Falloff_Distance_Min;
			float distance_delta = (distance - Shake_Falloff_Distance_Min);
			effect_delta = 1.0f - Math.Min(1.0f, distance_delta / range);
		}

		if (effect_delta <= 0.0f)
		{
			return;
		}
		
		// Buuuuuuuuuuzzzzzeeeee
		Chromatic_Arboration_Post_Process_FX fx = <Chromatic_Arboration_Post_Process_FX>Post_Process_FX.Get_Instance(typeof(Chromatic_Arboration_Post_Process_FX), Scene.Get_Camera_Index(this));
		fx.Show(chromatic_abberation_intensity * effect_delta);			
		
		m_shake_time = Math.Max(m_shake_time, duration * effect_delta);			
		m_shake_intensity += intensity * effect_delta;//Math.Max(m_shake_intensity, intensity * effect_delta);		

		if (m_shaking == false)
		{
			m_shake_current_time = 0.0f;
		}
		m_shaking = true;

		Scene.Vibrate(position, duration * 200.0f, intensity);
	}

	protected Vec4 Apply_Shake(Vec4 viewport)
	{
		if (!m_shaking)
		{
			return viewport;
		}

		float delta = 1.0f - (m_shake_current_time / m_shake_time);
		m_shake_current_time += Time.Get_Delta();

		float intensity = (m_shake_intensity * delta);
		
		// Finished shaking?
		if (delta <= 0.0f)
		{
			m_shaking = false;
			m_shake_intensity = 0.0;
			m_shake_time = 0.0;
		}

		// Target a random offset for shake.
		Vec2 offset = Vec2(
			Math.Rand(-intensity, intensity), 
			Math.Rand(-intensity, intensity)
		);

		// Calculate viewport offset by shake amount.
		Vec4 shaken_viewport = Vec4(
			viewport.X + offset.X,
			viewport.Y + offset.Y,
			viewport.Width,
			viewport.Height
		);

		// Lerp our viewport based on intensity
		float lerp_factor = Math.Clamp(Shake_Lerp_Factor * Time.Get_Delta(), 0.0f, 1.0f);
		viewport = Math.Lerp(viewport, shaken_viewport, lerp_factor);

		return viewport;
	}
}
