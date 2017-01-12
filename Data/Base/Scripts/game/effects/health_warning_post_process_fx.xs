// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;

public class Health_Warning_Post_Process_FX : Post_Process_FX
{	
	const float INTENSITY_LERP = 0.5f;
	const float HEARTBEAT_MAX_INTERVAL = 5.0f;
	const float HEARTBEAT_MIN_INTERVAL = 1.0f;
	
	const float MIN_INTENSITY = 0.5f;
	const float MAX_INTENSITY = 5.0f;
	const float MIN_INTENSITY_DAMAGE = 0.0f;
	const float MAX_INTENSITY_DAMAGE = 25.0f;	
	const float MAX_INTENSITY_LINGER_INTERVAL = 0.2f;
	const float UPPER_BOUND_INTENSITY_LINGER_INTERVAL = 1.5f;

	const float HEALTH_THRESHOLD = 0.35f;

	const float RED_INTENSITY_LERP = 0.55f;
	const float GREEN_INTENSITY_LERP = 0.55f;

	const float HARM_EFFECT_MULTIPLIER = 0.5f;
	const float HEAL_EFFECT_MULTIPLIER = 0.25f;

	float m_intensity = 1.0f;
	float m_target_intensity = 1.0f;

	float m_heartbeat_timer;

	float m_red_intensity_linger_timer = 0.0f;
	float m_red_intensity = 0.0f;

	float m_green_intensity_linger_timer = 0.0f;
	float m_green_intensity = 0.0f;

	Health_Warning_Post_Process_FX()
	{	
		Shader = "health_warning";
	}

	public void Show_Harm_Effect(float damage, bool override_linger = false)
	{
		damage *= HARM_EFFECT_MULTIPLIER;

		if (damage < MIN_INTENSITY_DAMAGE)
		{
			return;
		}

		float intensity_delta = (damage - MIN_INTENSITY_DAMAGE) / (MAX_INTENSITY_DAMAGE - MIN_INTENSITY_DAMAGE);
		
		// Always want at least half-red.
		intensity_delta = Math.Clamp(2.0 * intensity_delta, MIN_INTENSITY, MAX_INTENSITY);
		if (intensity_delta > m_red_intensity)	
		{
			m_red_intensity = intensity_delta;
		}
		
		if (override_linger = true)
		{
			m_red_intensity_linger_timer = 0.0f;
		}
		else
		{
			float new_linger = Math.Min(UPPER_BOUND_INTENSITY_LINGER_INTERVAL, MAX_INTENSITY_LINGER_INTERVAL * intensity_delta);
			if (new_linger > m_red_intensity_linger_timer)
			{
				m_red_intensity_linger_timer = new_linger;
			}
		}
	}

	public void Show_Heal_Effect(float damage, bool override_linger = false)
	{
		damage *= HEAL_EFFECT_MULTIPLIER;

		if (damage < MIN_INTENSITY_DAMAGE)
		{
			return;
		}

		float intensity_delta = (damage - MIN_INTENSITY_DAMAGE) / (MAX_INTENSITY_DAMAGE - MIN_INTENSITY_DAMAGE);
		
		// Always want at least half-red.
		intensity_delta = Math.Clamp(2.0 * intensity_delta, MIN_INTENSITY, MAX_INTENSITY);
		if (intensity_delta > m_green_intensity)	
		{
			m_green_intensity = intensity_delta;
		}
		
		if (override_linger = true)
		{
			m_green_intensity_linger_timer = 0.0f;
		}
		else
		{
			float new_linger = Math.Min(UPPER_BOUND_INTENSITY_LINGER_INTERVAL, MAX_INTENSITY_LINGER_INTERVAL * intensity_delta);
			if (new_linger > m_green_intensity_linger_timer)
			{
				m_green_intensity_linger_timer = new_linger;
			}
		}
	}

	public override void On_Shader_Setup()
	{
		Set_Shader_Uniform("g_intensity", m_intensity);
		Set_Shader_Uniform("g_red_intensity", m_red_intensity);
		Set_Shader_Uniform("g_green_intensity", m_green_intensity);
	}
	
	event void On_Tick()
	{
		float delta_t = Time.Get_Delta();
	
		float lowest_health_scalar = 1.0f;
		bool play_heartbeats = false;
		int living_users = 0;

		Game_Mode mode = Scene.Active_Game_Mode;

		// Only care about the camera we are attached to.
		int target_player_index = -1;
		if (mode != null && mode.Get_Camera_Count() > 1)
		{
			target_player_index = Camera_Index;
		}

		// Find lowest health value of all local players.
		NetUser[] users = Network.Get_Local_Users();				
		foreach (NetUser user in users)
		{
			Controller controller = user.Controller;
			Pawn pawn = null;
			
			if (target_player_index >= 0 && 
				user.Local_Index != target_player_index)
			{
				continue;
			}

			if (controller != null)
			{
				pawn = controller.Possessed;
				if (pawn != null && pawn.Can_Target())
				{
					float health_scalar = pawn.Display_Health / pawn.Max_Health;
					if (health_scalar < lowest_health_scalar)
					{
						lowest_health_scalar = health_scalar;
					}
					living_users++;
				}
			}
		}

		// If no users and we are not active, blah, do nothing.
		if (Active == false && living_users == 0)
		{
			return;
		}

		// If low enough set our new intensity target.
		if (lowest_health_scalar < HEALTH_THRESHOLD)
		{
			m_target_intensity = 1.0f;
			play_heartbeats = true;
		}
		else
		{
			m_target_intensity = 0.0f;
		}

		// Lerp to new target intensity.
		m_intensity = Math.Lerp(m_intensity, m_target_intensity, INTENSITY_LERP * delta_t);
		
		// Play heartbeat sound.
		if (m_target_intensity == 1.0f && play_heartbeats == true)
		{
			m_heartbeat_timer -= delta_t;

			if (m_heartbeat_timer <= 0.0f)
			{
				float health_delta = lowest_health_scalar / HEALTH_THRESHOLD;			
				Audio.Play2D("sfx_objects_avatars_heartbeat");

				m_heartbeat_timer = HEARTBEAT_MIN_INTERVAL + ((HEARTBEAT_MAX_INTERVAL - HEARTBEAT_MIN_INTERVAL) * health_delta);
				Show_Harm_Effect(MIN_INTENSITY_DAMAGE, true);
			}
		}

		// Fade out the red/green effects.
		m_red_intensity_linger_timer -= delta_t;
		m_green_intensity_linger_timer -= delta_t;

		if (m_red_intensity > 0.0f && m_red_intensity_linger_timer <= 0.0f)
		{
			m_red_intensity = Math.Lerp(m_red_intensity, 0.0f, RED_INTENSITY_LERP * delta_t);
		}

		if (m_green_intensity > 0.0f && m_green_intensity_linger_timer <= 0.0f)
		{
			m_green_intensity = Math.Lerp(m_green_intensity, 0.0f, GREEN_INTENSITY_LERP * delta_t);
		}
				
		// TODO: Fade out BGM volume.
		Audio.BGM_Volume_Multiplier = 1.0f - m_intensity;

		// Only active if any intensity > 0.0f;
		Active = (m_red_intensity > 0.01f || m_green_intensity > 0.01f || m_intensity > 0.01f);
	}
}
