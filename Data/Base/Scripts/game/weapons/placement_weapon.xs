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
	Name("Placement Weapon"), 
	Description("Base class for all weapons that need to be placed (turrets, etc).") 
]
public class Placement_Weapon : Weapon
{	
	// Object type to be spawned, must be derived from Movable.
	public Type Object_Type;

    // Maximum number of items the user can place.
    public int Max_Placed = -1;
	
	// Firing state.
	private bool m_played_dry_fire;
	private bool m_firing;
	private float m_fire_interval_timer;
	private float m_rof_multiplier;
	private float m_reload_multiplier;
	
	// Reloading state.
	private float m_recharge_accumulator;
	private float m_reload_timer;
	private float m_reload_duration;
	private float m_individual_reload_interval;
	private float m_individual_reload_next_shell_time;
	
	private int m_shots_fired;
	private int m_shots_fired_last;
	public int m_shots_fired_delta;
	
	public override int Get_Shots_Fired_Delta()
	{
		return m_shots_fired_delta;
	}

	// Gets animation state of weapon.
	public override string Get_Animation(Direction dir)
	{
		if (Walk_Animation != "")
		{
			return Walk_Animation + "_" + Direction_Helper.Mnemonics[dir];	
		}

		return "";
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
			Fire_Once();
		}
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
	}

    // Called when the movable for this weapon is spawned.
    protected virtual void On_Movable_Spawned(Movable movable)
    {

    }

	// Fires a single bullet.
	private void Fire_Once()
	{				
		// Enough time since last shot?
		if (m_fire_interval_timer > 0)
		{
			return;
		}
		 
		// Prevent firing for a while.
		m_fire_interval_timer = Fire_Interval * m_rof_multiplier;

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

			return;
		}
	
		// No can do, we are already carrying.
		if (Movable.Is_Pawn_Moving(this.Owner))
		{
			return;
		}

		// Spawn destroyable.
		if (Network.Is_Server())
		{
            // Destroy oldest placed actor if over maximum number placed.
            if (Max_Placed > 0)
            {
                Actor[] actors = Scene.Find_Actors(Object_Type);
                Movable oldest = null;
                int total_count = 0;

                foreach (Movable actor in actors)
                {
                    if (actor.Owner == this.Owner.Owner)
                    {
                        total_count++;

                        if (oldest == null || actor.Net_ID < oldest.Net_ID)
                        {
                            oldest = actor;
                        }
                    }
                }

                if (total_count >= Max_Placed)
                {
                    oldest.Force_Destroy();
                }
            }   

            // Place the new actor!
			Movable result			= <Movable>Scene.Spawn(Object_Type, this.Owner.Owner);
			result.Position			= this.Owner.Collision_Center;
			result.Layer			= this.Owner.Layer;
			result.Pickup_Pawn_ID	= this.Owner.Net_ID;
			result.Auto_Position	= true;
			result.Owner_ID			= this.Owner.Owner != null ? this.Owner.Owner.Net_ID : -1; 
			result.Team_ID			= this.Owner.Owner != null ? this.Owner.Owner.Team_Index : -1;
			result.Set_Ammo_Modifier(Ammo_Modifier);
            result.Set_Upgrade_Modifiers(Upgrade_Tree_Modifiers);

            On_Movable_Spawned(result);
		}
		
		// Count bullets fired.
		m_shots_fired++;
		
		// Play fire sound.	
		if (Fire_Sound != "")
		{
			Audio.Play3D(Fire_Sound, Owner.Position);
		}

		// Reduce ammo.
		Clip_Ammo -= Fire_Ammo_Usage;
	}	
}
