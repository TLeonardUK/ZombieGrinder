// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;
using game.actors.player.human_player;
 
public class Fire_Darken_Post_Process_FX : Post_Process_FX
{	
	private float m_target = 1.0f;
	private float m_tint = 1.0f;

	Fire_Darken_Post_Process_FX()
	{	
		Shader = ""; // We don't actually render any shader for this we just adjust map properties.
	}

	public override void On_Shader_Setup()
	{
	}
	
	public void Set_Target(float target)
	{
		m_target = target;
	}

	event void On_Tick()
	{
		m_tint = Math.Lerp(m_tint, m_target, 1.0f * Time.Get_Delta());
		m_target = 1.0f;

		Scene.Set_Map_Tint(Vec4(m_tint, m_tint, m_tint, 1.0f));

		Active = false;
	}
}
