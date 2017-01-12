// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.components.sprite;
using system.actors.actor;

[
	Placeable(true), 
	Name("Survivor"), 
	Description("SAAAAVEEE MEEEE!") 
]
public class Survivor : Actor, Interactable
{
	[ 
		Name("Cost"),
		Description("Coin cost if purchasable.")
	]
	public serialized(150) int Bonus = 100000;
	
	[ 
		EditType("Animation") 
	]
	public serialized(1) string Idle_Animation = "npc_survivor_v1_scared";

	[ 
		EditType("Animation") 
	]
	public serialized(1) string Rescue_Animation = "npc_survivor_v1_saved";

	public bool Rescued = false;
	
	private float m_ballon_alpha = 0.0f;
	private bool m_depth_bias_changed = false;

	Survivor()
	{
		Bounding_Box = Vec4(0, 0, 36, 41);
	}
	
	public override float Get_Duration(Pawn other)
	{
		return 250.0f;
	}
	
	public override int Get_Cost(Pawn other)
	{
		return 0;
	}
	
	public override string Get_Prompt(Pawn other)
	{
		return Locale.Get("#save_survivor_prompt");
	}
	
	public override bool Can_Interact(Pawn other)
	{
		if (Rescued)
		{
			return false;
		}

		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			return true;
		}
		return false;
	}

	public override void Interact(Pawn other)
	{
		Human_Player player = <Human_Player>other;
		if (player != null) 
		{
			rpc(RPCTarget.All, RPCTarget.None) RPC_Rescue();
		}
	}

	private rpc void RPC_Rescue()
	{
		Base_Game_Mode mode = <Base_Game_Mode>Scene.Active_Game_Mode;

		// Play save animation.
		sprite.Animation		= Rescue_Animation;
		sprite.Animation_Speed	= 1.0f;

		// Add score.
		mode.Add_Score(100000);//Bonus);
		FX.Spawn_Untracked("score_spray", Vec3(World_Bounding_Box.Center(), 0.0f), 0.0f, Layer, null, Bonus);

		Audio.Play3D("sfx_hud_achievement", this.Center);
			
		Log.Write("Rescued survivor " + this.Net_ID);

		Rescued = true;
	}
	
	default state Idle
	{
		event void On_Editor_Property_Change()
		{
			sprite.Animation		= Idle_Animation;
			sprite.Animation_Speed	= 1.0f;
		}
		event void On_Enter()
		{		
			sprite.Animation		= Idle_Animation;
			sprite.Animation_Speed	= 1.0f;
		}
		event void On_Tick()
		{
			if (Rescued)
			{
				float delta_t = Time.Get_Delta_Seconds();

				m_ballon_alpha += 255.0f * delta_t;
				
				if (!m_depth_bias_changed)
				{
					this.Layer = 4;
					m_depth_bias_changed = true;
				}

				if (m_ballon_alpha >= 255.0f)
				{
					ballon.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f);

					// Move upwards
					sprite.Offset = sprite.Offset + Vec2(0.0f, -50.0f * delta_t);
					ballon.Offset = ballon.Offset + Vec2(0.0f, -50.0f * delta_t);

					//if (sprite.Offset.Y > -8.0f)
					//{
					//}

					// Fade out after moving up a while.
					float fade_out_delay = 255 * 2;
					if (m_ballon_alpha > fade_out_delay)
					{
						float fade_delta = m_ballon_alpha - fade_out_delay;
						if (fade_delta >= 255.0f)
						{
							ballon.Color = Vec4(255.0f, 255.0f, 255.0f, 0.0f);
							sprite.Color = Vec4(255.0f, 255.0f, 255.0f, 0.0f);

							if (Network.Is_Server())
							{
								Log.Write("Despawning survivor " + this.Net_ID);
								Scene.Despawn(this);
							}
						}
						else
						{
							ballon.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f - fade_delta);
							sprite.Color = Vec4(255.0f, 255.0f, 255.0f, 255.0f - fade_delta);
						}
					}
				}
				else
				{
					ballon.Color = Vec4(255.0f, 255.0f, 255.0f, m_ballon_alpha);
				}
			}
		}
	}

	replicate(ReplicationPriority.Low)
	{
		Rescued
		{
			Owner = ReplicationOwner.Server;
			Mode  = ReplicationMode.Absolute;
		}
	}

	components
	{
		serialized(1) SpriteComponent sprite
		{
			Animation	= Idle_Animation;
			Color		= Vec4(255.0, 255.0, 255.0, 255.0);
		}
		SpriteComponent ballon
		{
			Frame		= "npc_survivor_ballon_0";
			Offset		= Vec2(-6.0f, -24.0f);
			Depth_Bias	= -32.0f;
			Color		= Vec4(255.0, 255.0, 255.0, 0.0);
		}
	}
}