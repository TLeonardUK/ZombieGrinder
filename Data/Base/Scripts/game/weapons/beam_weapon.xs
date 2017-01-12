// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.weapon;
using game.weapons.muzzle_flash;
using system.audio;
using system.time;
using system.fx;
using system.events;
using game.effects.fire_darken_post_process_fx; 

[
	Name("Beam Weapon"), 
	Description("Base class for all weapons the fire beams.") 
]
public class Beam_Weapon : Weapon
{	
	// Effect spawn while charging.
	public string Charge_Effect;

	// Effect spawn while firing.
	public string Beam_Effect;

	// Time required to charge.
	public float Charge_Time;

	// Duration to fire beam.
	public float Beam_Duration;

	// If set the weapon locks the users direction when firing.
	public bool Lock_Direction;

	// Firing state.
	private bool m_played_dry_fire;
	private bool m_firing;
	private float m_fire_interval_timer;
	private bool m_play_fire_anim;
	private float m_rof_multiplier;
	private float m_reload_multiplier;
	private Muzzle_Flash[] m_beam_effects = new Muzzle_Flash[0];
	
	// Reloading state.
	private float m_recharge_accumulator;
	private float m_reload_timer;
	private float m_reload_duration;
	private float m_individual_reload_interval;
	private float m_individual_reload_next_shell_time;

	// Charging state.
	private float m_charge_timer;
	private bool m_charging;
	private Audio_Channel m_charge_sound_channel;
	
	private int m_shots_fired;
	private int m_shots_fired_last;
	public int m_shots_fired_delta;

    Beam_Weapon()
    {
        // For the sake of AI input.
        Is_Automatic = true;
    }

	public override int Get_Shots_Fired_Delta()
	{
		return m_shots_fired_delta;
	}

	// Allows the pawn to lock the players direction.
	public override bool Is_Direction_Locked()
	{
		if (Lock_Direction == true)
		{
			return m_charging || m_firing;
		}
		else
		{
			return false;
		}
	}

	// Allows wepaon to prevent players interacting with objects.
	public override bool Is_Interact_Locked()
	{
		return Is_Direction_Locked();
	}

	// Kills all effects created by this weapon.
	public void Kill_Effects()
	{	
		// Hide old charge effects.
		Actor[] actors = Scene.Find_Actors(typeof(Muzzle_Flash));
		foreach (Muzzle_Flash flash in actors)
		{
			if (flash.Attached_Weapon == this &&
				(flash.Effect_Name == Charge_Effect || flash.Effect_Name == Beam_Effect))
			{
				flash.Hide();
				flash.Dispose();
			}
		}

        m_beam_effects.Clear();
	}

	// Begin chargign for shot.
	public void Begin_Charge()
	{
		m_charge_timer = 0.0f;
		m_charging = true;

		if (Charge_Effect != "")
		{
			Muzzle_Flash flash = <Muzzle_Flash>Scene.Spawn(typeof(Muzzle_Flash), null);
			flash.Parent = Owner;
			flash.Attached_Weapon = this;
			flash.Effect_Name = Charge_Effect;
		}
	}

	// End charging for shot.
	public void End_Charge(bool premature)
	{
		Kill_Effects();
		
		if (m_charge_sound_channel != null)
		{
			m_charge_sound_channel.Stop();
			m_charge_sound_channel = null;
		}

		if (premature == false)
		{
			Fire_Once();
		}

		m_charging = false;
	}

	// Update charging state.
	public void Update_Charge(float delta_t)
	{
		if (m_charging == true)
		{
			m_charge_timer += delta_t;

			float delta = (m_charge_timer / Charge_Time);
			if (delta >= 1.0f)
			{
				End_Charge(false);
			}
			else
			{
				if (m_charge_sound_channel != null)
				{
					m_charge_sound_channel.Position = Owner.Position;
				}
				else
				{				
					m_charge_sound_channel = Audio.Play3D(Charge_Sound, Owner.Position);
				}
			}
		}
	}
	
	// Gets animation state of weapon.
	public override string Get_Animation(Direction dir)
	{
		if (m_firing == true && !Is_Reloading && Clip_Ammo > 0) // Fix for reloading no ammo etc
		{
			if (Fire_Animation != "")
			{
				return Fire_Animation + "_" + Direction_Helper.Mnemonics[dir];	
			}
		}
		else
		{
			if (Walk_Animation != "")
			{
				return Walk_Animation + "_" + Direction_Helper.Mnemonics[dir];	
			}
		}

		return "";
 	}

	public override string Get_Oneshot_Animation(Direction dir)
	{
		if (m_play_fire_anim == true)
		{
			m_play_fire_anim = false;
			return Fire_Animation + "_" + Direction_Helper.Mnemonics[dir];	
		}
		return "";
 	}

	// Invoked when this weapon is unequipped.
	public override void On_Unequipped()
	{		
		Kill_Effects();
	}

	// Invoked when primary fire mode of this weapon begins.
	public override void On_Fire_Begin(float rof_multiplier, float reload_multiplier)	
	{	
		// If reloading we cannot fire!
		if (!Is_Reloading && !m_charging)
		{		
			m_rof_multiplier = rof_multiplier;
			m_reload_multiplier = reload_multiplier;
			
			// Check if we need to reload.
			if (Check_Ammo_For_Reload())
			{
				Begin_Charge();
			}
		}
	}
	
	// Invoked when primary fire mode of this weapon finishes.
	public override void On_Fire_End()
	{	
		// If we were charging kill charge effects.
		if (m_charging == true)
		{
			End_Charge(true);
		}

		m_charging = false;
	}

	// Invoked when reload is triggered.
	public override void On_Reload(float reload_multiplier)
	{
		// If we are charging or firing we cannot.
		if (m_charging == true || m_firing == true)
		{
			return;
		}

		// Reloading already?
		if (Is_Reloading == true)
		{
			return;
		}

		// Full ammo?
		if (Clip_Ammo == Clip_Size)
		{
			return;
		}

		// Rechargable weapons don't reload D:
		if (Is_Rechargable)
		{
			return;
		}
	
		// No ammo to reload? Blah, no point doing anything.
		if (Reserve_Ammo <= 0 && Has_Unlimited_Ammo == false)
		{
			return;
		}

		Is_Reloading = true;
		m_reload_timer = Reload_Duration * reload_multiplier;
		m_reload_duration = m_reload_timer;
		m_individual_reload_interval = m_reload_timer / <float>Clip_Size;
		m_individual_reload_next_shell_time = m_reload_timer - m_individual_reload_interval;
		
		if (Reload_Sound != "")
		{
			Audio.Play3D(Reload_Sound, Owner.Position);				
		}
	}

	// Returns the current progress of a reload (0..1).
	public override float Get_Reload_Delta()
	{
		return Math.Clamp((m_reload_duration - m_reload_timer) / m_reload_duration, 0.0f, 1.0f);
	}
	
	// Updates the reload state.
	private void Update_Reload(float delta)
	{
		if (Is_Rechargable == true)
		{
			m_recharge_accumulator += Recharge_Rate * delta;
			if (m_recharge_accumulator >= 1.0)
			{
				int to_add = <int>m_recharge_accumulator;
				Clip_Ammo = Math.Min(Clip_Size, Clip_Ammo + to_add);
				m_recharge_accumulator -= to_add;
			}
		}
		else
		{
			if (!Is_Reloading)
			{
				return;
			}
			
			m_reload_timer -= delta;

			bool finished = (m_reload_timer <= 0.0);
			
			// If we are reloading individual shells, pop one into the gun every so often.
			if (Use_Individual_Reload == true)
			{
				if (m_reload_timer <= m_individual_reload_next_shell_time)
				{
					if (Reload_Individual_Sound != "")
					{
						Audio.Play3D(Reload_Individual_Sound, Owner.Position);					
					}
					
					Clip_Ammo += 1;
					
					if (!Has_Unlimited_Ammo && Reserve_Ammo > 0)
					{
						Reserve_Ammo -= 1;
					}

					if ((Reserve_Ammo <= 0 && Has_Unlimited_Ammo == false) || Clip_Ammo >= Clip_Size)
					{
						finished = true;
					}
					
					m_individual_reload_next_shell_time -= m_individual_reload_interval;
				}
			}
			else
			{
				if (finished)
				{
					if (Has_Unlimited_Ammo == true)
					{
						Clip_Ammo = Clip_Size;
					}
					else
					{
						// Work out how much we can refill.
						int refill_ideal = Math.Min(Clip_Size - Clip_Ammo, Reserve_Ammo);		
						//refill_ideal = Clip_Size;
							
						Clip_Ammo = Math.Max(0, Clip_Ammo + refill_ideal);
						Reserve_Ammo = Math.Max(0, Reserve_Ammo - refill_ideal);
					}
				}
			}
						
			// Finished reloading yet?
			if (finished)
			{
				if (Reload_Finish_Sound != "")
				{
					Audio.Play3D(Reload_Finish_Sound, Owner.Position);			
				}
				Is_Reloading = false;
			}
		}
	}
	
	// Ticks the weapon!
	public override void On_Tick()
	{		
		float delta = Time.Get_Delta_Seconds();
		
		m_shots_fired_delta = m_shots_fired - m_shots_fired_last;
		m_shots_fired_last = m_shots_fired;
		
		// Reduce time until next fire.
		m_fire_interval_timer -= delta;
	
		// Reload shells.
		Update_Reload(delta);

		// Update the charging.
		Update_Charge(delta);

        // Update beam effect.
        foreach (Muzzle_Flash effect in m_beam_effects)
        {
            if (!Scene.Is_Spawned(effect))
            {
                m_beam_effects.Remove(effect);
                break;
            }
        }

        if (m_beam_effects.Length() <= 0)
        {
            m_firing = false;
        }

		// Keep shaking if firing.
		if (m_firing == true)
		{
			// Shake screen.		
			Camera.Shake_All(Screenshake_Duration, Screenshake_Intensity, Owner.Position, Owner, Chromatic_Arboration_Intensity);
		}
	}

	// Checks if we have enough ammo and reloads if not. Returns true if we can fire.
	private bool Check_Ammo_For_Reload()
	{			
		// Enough time since last shot?
		if (m_fire_interval_timer > 0)
		{
			
			Log.Write("Fire Intervasl: " + m_fire_interval_timer);

			return false;
		}
		 
		// Do we have ammo?
		if (Clip_Ammo < Fire_Ammo_Usage)// && Is_Rechargable == false)
		{	
			// Play dry-fire sound.
			if (Dry_Fire_Sound != "" && m_played_dry_fire == false)
			{
				Audio.Play3D(Dry_Fire_Sound, Owner.Position);
				m_played_dry_fire = true;
			}

			// Reload if we have any reserve ammo.
			if ((Reserve_Ammo > 0 || Has_Unlimited_Ammo) && !Is_Rechargable)
			{
				On_Reload(m_reload_multiplier);
			}

			return false;
		}
		
		return true;
	}

    private void Spawn_Muzzle_Flash(float directionOffset)
    {
        // Spawn muzzle effect.
		Muzzle_Flash effect     = <Muzzle_Flash>Scene.Spawn(typeof(Muzzle_Flash), null);
		effect.Parent           = Owner;
		effect.Attached_Weapon  = this;
		effect.Effect_Name      = Beam_Effect;
        effect.Direction_Offset  = directionOffset;
        
        m_beam_effects.AddLast(effect);

		// Count bullets fired.
		m_shots_fired++;
    }

	// Fires a single bullet.
	private void Fire_Once()
	{			
		// Check if we need to reload.
		if (!Check_Ammo_For_Reload())
		{
			return;
		}
		
		// Prevent firing for a while. 
		m_fire_interval_timer = Fire_Interval * m_rof_multiplier;
		m_firing = true;

        // Now to spawn the effects, how we do this depends on what modifiers we have attached.
        bool bStandardFire = true;
        int shotsFired = 0;
        if (Upgrade_Tree_Modifiers != null)
        {
            float largeFraction = Math.DegToRad(30.0f);
            float smallFraction = Math.DegToRad(10.0f);
            
            if (Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.QuadBarrel] != 0.0f)
            {
                if (Clip_Ammo >= Fire_Ammo_Usage * 4)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(-smallFraction);
                    Spawn_Muzzle_Flash(smallFraction);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 4;
                }
                else if (Clip_Ammo >= Fire_Ammo_Usage * 3)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(0);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 3;
                }
                else if (Clip_Ammo >= Fire_Ammo_Usage * 2)
                {
                    Spawn_Muzzle_Flash(-smallFraction);
                    Spawn_Muzzle_Flash(smallFraction);
                    shotsFired = 2;
                }
                else 
                {
                    Spawn_Muzzle_Flash(0);
                    shotsFired = 1;
                }
                bStandardFire = false;
            }
            else if (Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.TriBarrel] != 0.0f)
            {
                if (Clip_Ammo >= Fire_Ammo_Usage * 3)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(0);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 3;
                }
                else if (Clip_Ammo >= Fire_Ammo_Usage * 2)
                {
                    Spawn_Muzzle_Flash(-smallFraction);
                    Spawn_Muzzle_Flash(smallFraction);
                    shotsFired = 2;
                }
                else 
                {
                    Spawn_Muzzle_Flash(0);
                    shotsFired = 1;
                }
                bStandardFire = false;
            }
            else if (Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.MultiTap] != 0.0f)
            {
                 if (Clip_Ammo >= Fire_Ammo_Usage * 2)
                {
                    Spawn_Muzzle_Flash(-smallFraction);
                    Spawn_Muzzle_Flash(smallFraction);
                    shotsFired = 2;
                }
                else 
                {
                    Spawn_Muzzle_Flash(0);
                    shotsFired = 1;
                }
                bStandardFire = false;
            }
            else if (Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.Suicidal] != 0.0f)
            {
                int bulletsToFire = Math.Min(16, Clip_Ammo / Fire_Ammo_Usage);
                float angleStep = Math.PI2 / bulletsToFire; 
                for (int i = 0; i < bulletsToFire; i++)
                {
                   Spawn_Muzzle_Flash(i * angleStep);                 
                }
                shotsFired = bulletsToFire;
                bStandardFire = false;
            }
        }

        if (bStandardFire)
        {
            Spawn_Muzzle_Flash(0);
            shotsFired = 1;
        }

		// Play fire sound.	
		if (Fire_Sound != "")
		{
			Audio.Play3D(Fire_Sound, Owner.Position);
		}	
		
		// Let nearby objects know of this.
		if (Hear_Radius > 0.0f)
		{
			Event_Hear_Sound evt = new Event_Hear_Sound();
			evt.Instigator = Owner;
			evt.Fire_In_Radius(Owner.Center, Hear_Radius);
		}

		// Reduce ammo.
		Clip_Ammo -= Fire_Ammo_Usage * shotsFired;
	}	
}
