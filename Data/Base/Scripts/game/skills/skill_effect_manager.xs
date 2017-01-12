// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
 
public class Active_Skill_Effect
{
	public float			Time_Remaining = 0.0f;
	public int				Instigator_ID  = 0;
	public int				Team_ID        = 0;
	public Skill_Archetype	Archetype	   = null;
	public bool				Created_Effects = false;
}

[
	Name("Skill Effect Manager"), 
	Description("Responsible for applying the effects of different skills.") 
]
public static class Skill_Effect_Manager
{
	public static bool m_reset_after_inactive = false;

	private static Active_Skill_Effect[] m_active_effects = new Active_Skill_Effect[0];

	// Removes all active skill effects.
	public static void Reset()
	{
		m_active_effects = new Active_Skill_Effect[0];
		m_reset_after_inactive = false;
	}
	
	// Updates all skill effects.
	public static void Tick()
	{
		float delta_t = Time.Get_Delta_Seconds();
		
		NetUser[] users = Network.Get_Users();

		// Non-active?
		if (m_active_effects.Length() <= 0)
		{		
			if (m_reset_after_inactive == false)
			{
				// Reset multipliers to base.
				foreach (NetUser user in users)
				{
					user.Reset_Skill_Multipliers();
				}

				m_reset_after_inactive = true;
			}

			return;
		}
		else
		{
			m_reset_after_inactive = false;
		}

		// Reset multipliers to base.
		foreach (NetUser user in users)
		{
			user.Reset_Skill_Multipliers();
		}

		// Apply active multipliers.
		foreach (NetUser user in users)
		{
			int netid = user.Net_ID;
			int teamid = user.Team_Index;

			if (user.Controller != null && user.Controller.Possessed != null)
			{
				Human_Player player = <Human_Player>user.Controller.Possessed;
                if (player)
                {
                    if (player.Skills_Disabled)
                    {
                        continue;
                    }
                }
            }

			foreach (Active_Skill_Effect effect in m_active_effects)
			{
				if (effect.Instigator_ID == netid || (effect.Team_ID == teamid && effect.Archetype.Is_Team_Based == true))
				{
					if (effect.Created_Effects == false && effect.Archetype.Player_Effect != "")
					{
						if (user.Controller != null && user.Controller.Possessed != null)
						{
							Human_Player player = <Human_Player>user.Controller.Possessed;
                            if (player)
                            {
		    					Effect f = FX.Spawn(effect.Archetype.Player_Effect, player.Center, 0, player.Layer, player, 0, "", 0);
	    						f.Attach_To_Offset(player, Vec3(0, 8, 0));//player.Center - player.Position);
    							f.DisposeOnFinish = true;
                            }
						}
					}

					effect.Archetype.Apply(user);
				}
			}
		}
		foreach (Active_Skill_Effect effect in m_active_effects)
		{
			effect.Created_Effects = true;
		}

		// Get rid of worn off effects.
		bool removed = true;
		while (removed)
		{
			removed = false;

			foreach (Active_Skill_Effect effect in m_active_effects)
			{
				effect.Time_Remaining -= delta_t;
				if (effect.Time_Remaining <= 0.0f)
				{
					Log.Write("Active effect '"+Locale.Get(effect.Archetype.Name)+"' has worn off");
					m_active_effects.Remove(effect);
					removed = true;
					break;
				}
			}
		}
	}

	// Adds a new skill effect.
	public static void Activate(Skill_Archetype archetype, NetUser instigator)
	{
		Log.Write("User "+instigator.Username+" activated skill '"+Locale.Get(archetype.Name)+"'");

		Active_Skill_Effect skill = new Active_Skill_Effect();
		skill.Time_Remaining  = archetype.Duration;
		skill.Instigator_ID   = instigator.Net_ID;
		skill.Team_ID		  = instigator.Team_Index;
		skill.Archetype       = archetype;
		skill.Created_Effects	= false;

		m_active_effects.AddLast(skill);
	}

	// Returns true if a skill is active for the given user.
	public static bool Is_Active(Skill_Archetype archetype, NetUser instigator)
	{
		if (m_active_effects.Length() <= 0)
		{
			return false;
		}

		int netid = instigator.Net_ID;
		int teamid = instigator.Team_Index;

		foreach (Active_Skill_Effect effect in m_active_effects)
		{
			if (effect.Archetype == archetype)
			{
				if (effect.Instigator_ID == netid || (effect.Team_ID == teamid && effect.Archetype.Is_Team_Based == true))
				{
					return true;
				}
			}
		}

		return false;
	}

	// Returns time left for active effect.
	public static float Get_Time_Left(Skill_Archetype archetype, NetUser instigator)
	{
		if (m_active_effects.Length() <= 0)
		{
			return 0.0f;
		}

		int netid = instigator.Net_ID;
		int teamid = instigator.Team_Index;

		foreach (Active_Skill_Effect effect in m_active_effects)
		{
			if (effect.Archetype == archetype)
			{
				if (effect.Instigator_ID == netid || (effect.Team_ID == teamid && effect.Archetype.Is_Team_Based == true))
				{
					return effect.Time_Remaining;
				}
			}
		}

		return 0.0f;
	}
}