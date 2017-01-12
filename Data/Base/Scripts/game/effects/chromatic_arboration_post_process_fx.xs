// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;

public class Chromatic_Arboration_Post_Process_FX : Post_Process_FX
{	
	Vec2 m_intensity;
	float m_target_intensity;
	
	Chromatic_Arboration_Post_Process_FX()
	{	
		Shader = "chromatic_arboration";
		m_intensity = Vec2(0.0f, 0.0f);
	}

	public override void On_Shader_Setup()
	{
		Set_Shader_Uniform("g_intensity", Vec3(m_intensity, 0.0f));
	}
	
	public float Show(float intensity)
	{
		m_target_intensity = intensity;
	}
	
	event void On_Tick()
	{		
		m_target_intensity = Math.Lerp(m_target_intensity, 0.0f, 5.0f * Time.Get_Delta_Seconds());

		float wobble_factor = Math.Max(0.0f, m_target_intensity - 1.0f) * 0.5f;
		float angle = Math.Rand(0.0f, Math.PI2);
		m_intensity = Vec2(m_target_intensity, 0.0f) + Vec2(Math.Sin(angle) * wobble_factor, Math.Cos(angle) * wobble_factor);
		
		Active = (m_intensity.X >= 1.0f || m_intensity.Y >= 1.0f);
	}
}
