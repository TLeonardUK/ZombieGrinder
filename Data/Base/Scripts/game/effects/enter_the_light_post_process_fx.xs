// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.post_process_fx;
using system.time;
using system.audio;
using system.network;
using game.actors.player.human_player;

public class Enter_The_Light_Post_Process_FX : Post_Process_FX
{	
	float m_intensity = 1.0f;
	float m_dark_intensity = 1.0f;

	Enter_The_Light_Post_Process_FX()
	{	
		Shader = "enter_the_light";
	}

	public override void On_Shader_Setup()
	{
		Set_Shader_Uniform("p_intensity", m_intensity);
		Set_Shader_Uniform("p_dark_intensity", m_dark_intensity);
	}
	
	event void On_Tick()
	{
		float delta_t = Time.Get_Delta();
		int living_players = 0;
		int ghost_players = 0;
		float highest_bleedout = 0.0f;
		Game_Mode mode = Scene.Active_Game_Mode;

		// Only care about the camera we are attached to.
		int target_player_index = -1;
		if (mode != null && mode.Get_Camera_Count() > 1)
		{
			target_player_index = Camera_Index;
		}

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
				if (pawn != null)
				{
					if (pawn.Can_Target())
					{
						living_players++;
					}
					else
					{
						Human_Player player = <Human_Player>pawn;
						if (player != null)
						{
							ghost_players++;
							
							float bleedout = player.Get_Incapacitated_Bleedout_Factor();
							if (bleedout > highest_bleedout)
							{
								highest_bleedout = bleedout;
							}
						}
					}
				}
			}
		}

		if (ghost_players > 0 && living_players <= 0)
		{
			m_intensity	= highest_bleedout;

			if (m_intensity >= 0.5f)
			{
				float elapsed = (m_intensity - 0.5f) / 0.5f;
				m_dark_intensity = 1.0f - elapsed;
			}
			else
			{
				m_dark_intensity = 1.0f;		
			}
		}
		else
		{
			m_intensity = 0;
			m_dark_intensity = 0.0f;
		}

		// Only active if any intensity > 0.0f;
		Active = (m_dark_intensity > 0.01f || m_intensity > 0.01f);
	}
}
