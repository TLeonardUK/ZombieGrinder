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
	Name("Projectile Weapon"), 
	Description("Base class for all weapons the fire projectiles.") 
]
public class Projectile_Weapon : Weapon
{	
	// Effect spawned at the muzzle when this projectile is spawned.
	public string Muzzle_Effect;
	
	// Darken map background while firing weapon (useful for lighten particlefx).
	public bool Darken_Map;
	public float Darken_Map_Multiplier = 1.0f;
	
	// Firing state.
	private bool m_played_dry_fire;
	private bool m_firing;
	private float m_fire_interval_timer;
	private bool m_play_fire_anim;
	private bool m_play_fire_anim_tint;
	private Audio_Channel m_fire_sound_channel;
	private float m_rof_multiplier = 1.0f;
	private float m_reload_multiplier = 1.0f;
	
	private int m_shots_fired;
	private int m_shots_fired_last;
	public int m_shots_fired_delta;

	// Reloading state.
	private float m_recharge_accumulator;
	private float m_reload_timer;
	private float m_reload_duration;
	private float m_reload_duration_timer;
	private float m_individual_reload_interval;
	private float m_individual_reload_next_shell_time;
	
	public override int Get_Shots_Fired_Delta()
	{
		return m_shots_fired_delta;
	}

	public override bool Is_Firing()
	{
		return m_firing;
	}

	// Gets animation state of weapon.
	public override string Get_Animation(Direction dir)
	{
		if (Is_Automatic == true && m_firing == true && !Is_Reloading && Clip_Ammo > 0) // Fix for reloading no ammo etc
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
	
	public override string Get_Tint_Animation(Direction dir)
	{
		if (Is_Automatic == true && m_firing == true && !Is_Reloading && Clip_Ammo > 0) // Fix for reloading no ammo etc
		{
			if (Fire_Animation != "")
			{
				return Fire_Animation + "_tint_" + Direction_Helper.Mnemonics[dir];	
			}
		}
		else
		{
			if (Walk_Animation != "")
			{
				return Walk_Animation + "_tint_" + Direction_Helper.Mnemonics[dir];	
			}
		}

		return "";
 	}

	public override string Get_Oneshot_Tint_Animation(Direction dir)
	{
		if (m_play_fire_anim_tint == true)
		{
			m_play_fire_anim_tint = false;
			return Fire_Animation + "_tint_" + Direction_Helper.Mnemonics[dir];	
		}
		return "";
 	}

	// Invoked when this weapon is unequipped.
	public override void On_Unequipped()
	{			
		// Discard old muzzle flashes.
		/*Actor[] actors = Scene.Find_Actors(typeof(Muzzle_Flash));
		foreach (Muzzle_Flash flash in actors)
		{
			if (flash.Attached_Weapon == this)
			{
				flash.Dispose();
			}
		}*/
	}

	// Invoked when primary fire mode of this weapon begins.
	public override void On_Fire_Begin(float rof_multiplier, float reload_multiplier)	
	{	
		// If reloading we cannot fire!
		if (!Is_Reloading)
		{		
			m_rof_multiplier = rof_multiplier;
			m_reload_multiplier = reload_multiplier;

			// Not automatic just fire once, otherwise track firing so we 
			// can spam out some bullets.
			if (Is_Automatic == false)
			{
				Fire_Once();
			}
			else
			{
				m_played_dry_fire = false;
				m_firing = true;
			}
		}
	}
	
	// Invoked when primary fire mode of this weapon finishes.
	public override void On_Fire_End()
	{	
		m_firing = false;
		
		if (m_fire_sound_channel != null)
		{
			m_fire_sound_channel.Stop();
			m_fire_sound_channel = null;
		}
	}
	
	// Invoked when secondary fire mode of this weapon begins.
	public override void On_Alt_Fire_Begin(float rof_multiplier, float reload_multiplier)	
	{	
		FX.Invoke_Game_Trigger(Owner);
	}

	// Invoked when reload is triggered.
	public override void On_Reload(float reload_multiplier)
	{
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

		// Reload multiplier.
		float sub_reload_multiplier = 1.0f;
		if (this.Owner != null && this.Owner.Owner != null)
		{
			sub_reload_multiplier = this.Owner.Owner.Stat_Multipliers[Profile_Stat.Reload_Speed];
		}
        
        // Weapon upgrade modifiers.
        if (Upgrade_Tree_Modifiers != null && 
            Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed] != 0.0f)
        {
            float multiplier = 1.0f / (1.0f + Math.Max(Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed], -0.95f));
            sub_reload_multiplier *= multiplier;
        }

		Is_Reloading = true;
		m_reload_timer = Reload_Duration * reload_multiplier * sub_reload_multiplier;
		m_individual_reload_interval = m_reload_timer / <float>Clip_Size;
		m_individual_reload_next_shell_time = m_reload_timer - m_individual_reload_interval;
				
		if (Use_Individual_Reload == true)
		{
			m_reload_duration = m_individual_reload_interval * (Clip_Size - Clip_Ammo);
			m_reload_duration_timer = m_reload_duration;
		}
		else
		{
			m_reload_duration = m_reload_timer;
			m_reload_duration_timer = m_reload_duration;
		}

		if (Reload_Sound != "")
		{
			Audio.Play3D(Reload_Sound, Owner.Position);				
		}
	}

	// Returns the current progress of a reload (0..1).
	public override float Get_Reload_Delta()
	{
		return Math.Clamp((m_reload_duration - m_reload_duration_timer) / m_reload_duration, 0.0f, 1.0f);
	}

	// Updates the reload state.
	private void Update_Reload(float delta)
	{
		if (Is_Rechargable == true)
		{
            // Weapon upgrade modifiers.
            float recharge_rate = Recharge_Rate;
            if (Upgrade_Tree_Modifiers != null && 
                Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed] != 0.0f)
            {
                float multiplier = 1.0f / (1.0f + Math.Max(Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed], -0.95f));
                recharge_rate *= multiplier;
            }

			m_recharge_accumulator += recharge_rate  * delta;
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
			m_reload_duration_timer -= delta;

			bool finished = (m_reload_timer <= 0.0);

			float ammo_multiplier = 1.0f;
			if (this.Owner != null && this.Owner.Owner != null)
			{
				ammo_multiplier = this.Owner.Owner.Stat_Multipliers[Profile_Stat.Ammo_Usage];
			}
			bool is_unlimited = (Has_Unlimited_Ammo == true || ammo_multiplier == 0.0f) && (Ammo_Modifier == null);

			// If we are reloading individual shells, pop one into the gun every so often.
			if (Use_Individual_Reload == true)
			{
				if (m_reload_timer <= m_individual_reload_next_shell_time)
				{
					if (Reload_Individual_Sound != "")
					{
						Audio.Play3D(Reload_Individual_Sound, Owner.Position);					
					}

                    if (Reserve_Ammo > 0 || is_unlimited)
                    {
                        Clip_Ammo += 1;
                    }
					
					// Reload multiplier.
					if (!is_unlimited && Reserve_Ammo > 0)
					{
						Reserve_Ammo -= 1;
					}

					if ((Reserve_Ammo <= 0 && is_unlimited == false) || Clip_Ammo >= Clip_Size)
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
					if (is_unlimited)
					{
						Clip_Ammo = Clip_Size;
					}
					else
					{
						// Work out how much we can refill.
						int refill_ideal = Math.Min(Clip_Size - Clip_Ammo, Reserve_Ammo);		
							
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
		
		// Currently firing? Try and spawn next bullet.
		if (m_firing == true)
		{			
			if (Darken_Map == true && Is_Reloading == false && (Clip_Ammo > 0 || Reserve_Ammo > 0))
			{
				Fire_Darken_Post_Process_FX fx = <Fire_Darken_Post_Process_FX>Post_Process_FX.Get_Instance(typeof(Fire_Darken_Post_Process_FX), this.Owner.Owner != null ? this.Owner.Owner.Local_Index : 0);
				fx.Set_Target(Darken_Map_Multiplier);
			}
			
			Fire_Once();
		}

		// Get rid of ammo mod when out of bullets.
		if (Ammo_Modifier != null)
		{
			if (Clip_Ammo <= 0 && Reserve_Ammo <= 0)
			{
				Ammo_Modifier = null;
			}
		}
	}

    public Muzzle_Flash Spawn_Muzzle_Flash(float directionOffset)
    {
        // Spawn muzzle effect.
		Muzzle_Flash flash		        = <Muzzle_Flash>Scene.Spawn(typeof(Muzzle_Flash), null);
		flash.Parent			        = Owner;
		flash.Attached_Weapon	        = this;
		flash.Effect_Name		        = Muzzle_Effect;
		flash.Effect_Modifier	        = (Ammo_Modifier != null ? Ammo_Modifier.Effect_Modifier : ""); 	 	
		flash.Ignore_Spawn_Collision    = Ignore_Spawn_Collision;
        flash.Direction_Offset          = directionOffset;

		// Count bullets fired.
		m_shots_fired++;

        return flash;
    }

    public bool Begin_Fire_Spawn()
    {        			
		// Enough time since last shot?
		if (m_fire_interval_timer > 0)
		{
			return false;
		}

		// Reload multiplier.
		float sub_rof_multiplier = 1.0f;
		if (this.Owner != null && this.Owner.Owner != null)
		{
			sub_rof_multiplier = this.Owner.Owner.Stat_Multipliers[Profile_Stat.Rate_Of_Fire];
		}
		
		float ammo_multiplier = 1.0f;
		if (this.Owner != null && this.Owner.Owner != null)
		{
			ammo_multiplier = this.Owner.Owner.Stat_Multipliers[Profile_Stat.Ammo_Usage];
		}
		bool is_unlimited = (Has_Unlimited_Ammo == true || ammo_multiplier == 0.0f) && (Ammo_Modifier == null);

        // Weapon upgrade modifiers.
        if (Upgrade_Tree_Modifiers != null && 
            Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.RateOfFire] != 0.0f)
        {
            float multiplier = 1.0f / (1.0f + Math.Max(Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.RateOfFire], -0.95f));
            sub_rof_multiplier *= multiplier;
        }
        
		// Prevent firing for a while.
		m_fire_interval_timer = Fire_Interval * m_rof_multiplier * sub_rof_multiplier;
        
		// Do we have ammo?
		bool bHasAmmo = false;
		Pawn ownerPawn = <Pawn>this.Owner;
		if (Stamina_Based && ownerPawn != null)
		{
			bHasAmmo = (ownerPawn.Stamina >= Fire_Ammo_Usage);
		}
		else
		{
			bHasAmmo = (Clip_Ammo >= Fire_Ammo_Usage);
		}

		if (!bHasAmmo)// && Is_Rechargable == false)
		{	
			// Stop firing sound.			
			if (m_fire_sound_channel != null)
			{
				m_fire_sound_channel.Stop();
				m_fire_sound_channel = null;
			}

			// Play dry-fire sound.
			if (Dry_Fire_Sound != "" && m_played_dry_fire == false)
			{
				Audio.Play3D(Dry_Fire_Sound, Owner.Position);
				m_played_dry_fire = true;
			}

			// Reload if we have any reserve ammo.
			if ((Reserve_Ammo > 0 || is_unlimited) && !Is_Rechargable)
			{
				On_Reload(m_reload_multiplier);
			}
			
			return false;
		}
        
        return true;
    }

    public void End_Fire_Spawn(int shotsFired)
    {
		Pawn ownerPawn = <Pawn>this.Owner;

		// Play fire sound.	
		if (Fire_Loop_Sound == "")
		{
			if (Fire_Sound != "")
			{
				Audio.Play3D(Fire_Sound, Owner.Position);
			}
		}
		else
		{
			if (m_fire_sound_channel != null)
			{
				m_fire_sound_channel.Position = Owner.Position;
			}
			else
			{				
				if (Fire_Loop_Start_Sound != "")
				{
					Audio.Play3D(Fire_Loop_Start_Sound, Owner.Position);
				}

				m_fire_sound_channel = Audio.Play3D(Fire_Loop_Sound, Owner.Position);
			}
		}
		
		// Shake screen.		
		Camera.Shake_All(Screenshake_Duration, Screenshake_Intensity, Owner.Position, Owner);
		
		// Let nearby objects know of this.
		if (Hear_Radius > 0.0f)
		{
			Event_Hear_Sound evt = new Event_Hear_Sound();
			evt.Instigator = Owner;
			evt.Fire_In_Radius(Owner.Center, Hear_Radius);
		}

		// Reduce ammo.
		if (Stamina_Based && ownerPawn != null)
		{
			ownerPawn.Stamina -= Fire_Ammo_Usage * shotsFired;
		}
		else
		{
			Clip_Ammo -= Fire_Ammo_Usage * shotsFired;
		}

		// Queue oneshot fire anim.
		if (Is_Automatic == false)
		{
			m_play_fire_anim = true;
			m_play_fire_anim_tint = true;
		}
    }

	// Fires a single bullet.
	private void Fire_Once()
	{
        if (!Begin_Fire_Spawn())
        {
            return;
        }

		Pawn ownerPawn = <Pawn>this.Owner;

        // Now to spawn the effects, how we do this depends on what modifiers we have attached.
        bool bStandardFire = true;
        int shotsFired = 0;
        if (Upgrade_Tree_Modifiers != null)
        {
            float largeFraction = Math.DegToRad(30.0f);
            float smallFraction = Math.DegToRad(10.0f);
            
            if (Upgrade_Tree_Modifiers[<int>Item_Upgrade_Tree_Node_Modifier_Type.QuadBarrel] != 0.0f)
            {
                if (Clip_Ammo > 3)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(-smallFraction);
                    Spawn_Muzzle_Flash(smallFraction);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 4;
                }
                else if (Clip_Ammo > 2)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(0);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 3;
                }
                else if (Clip_Ammo > 1)
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
                if (Clip_Ammo > 2)
                {
                    Spawn_Muzzle_Flash(-largeFraction);
                    Spawn_Muzzle_Flash(0);
                    Spawn_Muzzle_Flash(largeFraction);
                    shotsFired = 3;
                }
                else if (Clip_Ammo > 1)
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
                 if (Clip_Ammo > 1)
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
                int bulletsToFire = Math.Min(16, Clip_Ammo);
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

        End_Fire_Spawn(1);
	}	
}
