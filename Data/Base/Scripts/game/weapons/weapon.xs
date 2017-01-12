// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log; 
using system.actors.pawn;
using game.profile.profile;
using game.weapons.weapon_ammo;

public enum Weapon_Slot
{
	Pistol,
	Melee,
	Shotgun,
	Rifle,
	Heavy,
	Grenade,
	COUNT
}
 
public enum Weapon_SubType
{
	Melee,
	Heavy,
	Turret,
	Small_Arms,
	Rifle,
	Grenade,
	Healing,
	Enemy,
	COUNT
}
 
[
	Name("Weapon"), 
	Description("Base class for all weapons the player or enemies can use.") 
]
public class Weapon
{	
	// Display name for weapon.
	public string Name;

	// Animation used when walking with this weapon. Should be the prefix of the direction based animations.
	public string Walk_Animation;
	
	// Animation used when firing this weapon.
	public string Fire_Animation;
	
	// Layer based on direction 1 = above head, 0 = below head.
									//S  SE E  NE N  NW W  SW
	public int[] Direction_Layers = { 1, 1, 1, 0, 0, 0, 1, 1 };
	
	// Muzzle position by direction.
	public Vec2[] Muzzle_Position = {
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0),
		new Vec2(0, 0)
	};  
	
	// What ammo types this weapon can take.
	public Type[] Accepted_Ammo_Types = null;

	// Speed multipler for pawn holding this weapon.
	public float Speed_Multiplier;
	
	// Inventory slot weapon should reside in.
	public Weapon_Slot Slot;

	// Weapon sub-type, used when applying stats.
	public Weapon_SubType SubType;
	
	// Will cause the particles emitter by this effect to ingore any collision they are spawned within.
	public bool Ignore_Spawn_Collision = false;

	// Max ammo amounts.
	public serialized(1) int Clip_Size;
	public serialized(1) int Reserve_Size;
	public int Initial_Ammo;
	public serialized(1) bool Has_Unlimited_Ammo;
	public float Reload_Duration;			// in seconds
	public bool Use_Individual_Reload;
	public float Fire_Interval;				// in seconds
	public int Is_Rechargable;
	public float Recharge_Rate; 			// in bullets per second.
	public float Recharge_Interval;			// in seconds
	public int Fire_Ammo_Usage;
	
	public bool Can_Buffs_Regenerate_Ammo = true;
	
	public bool Stamina_Based = false;

	public bool Can_Alt_Fire = false;

    // Upgrade stats.
    public int[]            Upgrade_Ids;
    public Item_Archetype   Upgrade_Item_Archetype;

    public serialized(1) float[] Upgrade_Tree_Modifiers;

	// Current ammo state.
	public serialized(1) Weapon_Ammo Ammo_Modifier;
	public serialized(1) int Reserve_Ammo;
	public serialized(1) int Clip_Ammo;	

	public float Ammo_Regen_Accumulator;

	public bool Is_Reloading;
    
	// Automatic or semiautomatic?
	public bool Is_Automatic;

	// Primary rendering color. Used for tints on things like the fight-sabar.
	public bool Is_Tintable;
	public serialized(1) Vec4 Primary_Color;

	// Screenshake on fire.
	public float Screenshake_Duration;		// in seconds
	public float Screenshake_Intensity;
	
	// Intensity of chromatic arboration when firing.
	public float Chromatic_Arboration_Intensity = 0.0f;
	
	// HUD Information.
	public string HUD_Icon;
	public string HUD_Ammo_Icon;
	
	// Sounds.
	public string Charge_Sound;
	public string Fire_Sound;
	public string Fire_Loop_Start_Sound;
	public string Fire_Loop_Sound;
	public string Dry_Fire_Sound;
	public string Reload_Sound;
	public string Reload_Individual_Sound;
	public string Reload_Finish_Sound;

    // Manual drop pickup information.
    public string Pickup_Sprite;
    public string Pickup_Sound;
    public string Pickup_Name;

	// AI audio propogation.
	public float Hear_Radius;

	// AI Priority to use this weapon. The higher the priority
	// the more the AI will want to use it.
	public float AI_Priority;

	// Pawn that owns this weapon.
	public Actor Owner;

    // Item type of weapon.
    // WARNING: This is the base archetype of a weapon (eg a speedy uzi, will return the uzi archetype).
    public Item_Archetype Item_Type;
	
	// Gets animation state of weapon.
	public virtual string Get_Animation(Direction dir)
	{
		return "";
	}

	public virtual string Get_Oneshot_Animation(Direction dir)
	{
		return "";
	}
	
	public virtual string Get_Tint_Animation(Direction dir)
	{
		return "";
	}

	public virtual string Get_Oneshot_Tint_Animation(Direction dir)
	{
		return "";
	}

	// Firing state.
	public virtual int Get_Shots_Fired_Delta()
	{
		return 0;
	}

	// Invoked when this weapon is unequipped.
	public virtual void On_Unequipped()
	{	
		// Use in derived class.
	}
	
	// Invoked when this weapon is equipped.
	public virtual void On_Equipped()
	{	
		// Use in derived class.
	}
	
	// Invoked when primary fire mode of this weapon begins.
	public virtual void On_Fire_Begin(float rof_multiplier, float reload_multiplier)
	{	
		// Use in derived class.
	}
	
	// Invoked when primary fire mode of this weapon finishes.
	public virtual void On_Fire_End()
	{	
		// Use in derived class.
	}
	
	// Invoked when secondary fire mode of this weapon begins.
	public virtual void On_Alt_Fire_Begin(float rof_multiplier, float reload_multiplier)
	{	
		// Use in derived class.
	}
	
	// Invoked when secondary fire mode of this weapon finishes
	public virtual void On_Alt_Fire_End()
	{	
		// Use in derived class.
	}
	
	// Invoked when reload is triggered.
	public virtual void On_Reload(float reload_multiplier)
	{
		// Use in derived class.	
	}

	// Returns the current progress of a reload (0..1).
	public virtual float Get_Reload_Delta()
	{
		return 0.0f;
	}
	
	// Invoked when the weapon is active.
	public virtual void On_Tick()
	{
		// Use in derived class.	
	}
	
	// Allows weapons to lock the pawns direction.
	public virtual bool Is_Direction_Locked()
	{
		return false;
	}
	
	// Allows wepaon to prevent players interacting with objects.
	public virtual bool Is_Interact_Locked()
	{
		return false;
	}

	// Returns true if we are currently firing.
	public virtual bool Is_Firing()
	{
		return false;
	}

	// Do we have any ammo?
	public virtual bool Has_Ammo()
	{
		return (Reserve_Ammo > 0 || Clip_Ammo > 0) || Has_Unlimited_Ammo;
	}

	// Fills the weapons ammo to capacity.
	public virtual void Fill_Ammo()
	{	
		Reserve_Ammo = Reserve_Size;
		Clip_Ammo = Clip_Size;

		if (this.Ammo_Modifier != null)
		{
			int max_ammo = Reserve_Size + Clip_Size;
			int max_mod_ammo = <int>Math.Max(<float>Math.Min(5, max_ammo), max_ammo * this.Ammo_Modifier.Clip_Size_Scalar); // Always allow 5 or so bullets.
			
			int mod_clip_size = Math.Min(Clip_Size, max_mod_ammo);
			int mod_reserve_size = Math.Max(0, max_mod_ammo - mod_clip_size);
			
			Clip_Ammo = mod_clip_size;
			Reserve_Ammo = mod_reserve_size;
		}
	}
	
	// Fills the weapons reserve ammo.
	public virtual void Fill_Reserve_Ammo()
	{	
		Reserve_Ammo = Reserve_Size;

		if (this.Ammo_Modifier != null)
		{
			int max_ammo = Reserve_Size + Clip_Size;
			int max_mod_ammo = <int>Math.Max(<float>Math.Min(5, max_ammo), max_ammo * this.Ammo_Modifier.Clip_Size_Scalar); // Always allow 5 or so bullets.
			
			int mod_clip_size = Math.Min(Clip_Size, max_mod_ammo);
			int mod_reserve_size = Math.Max(0, max_mod_ammo - mod_clip_size);

			Reserve_Ammo = mod_reserve_size;
		}
	}
	
	// Changes the modifier currently applied to the weapons ammo.
	public virtual void Change_Ammo_Modifier(Weapon_Ammo ammo)
	{
		Ammo_Modifier = ammo;
		if (ammo != null)
		{
			int max_ammo = Reserve_Size + Clip_Size;
			int max_mod_ammo = <int>Math.Max(<float>Math.Min(5, max_ammo), max_ammo * ammo.Clip_Size_Scalar); // Always allow 5 or so bullets.

			int mod_clip_size = Math.Min(Clip_Size, max_mod_ammo);
			int mod_reserve_size = Math.Max(0, max_mod_ammo - mod_clip_size);

			Clip_Ammo = mod_clip_size;
			Reserve_Ammo = mod_reserve_size;
		}
	}

	// Returns true if we can accept the given ammo type.
	public virtual bool Can_Accept_Weapon_Modifier(Type ammo_type)
	{
		if (Accepted_Ammo_Types != null)
		{
			foreach (Type t in Accepted_Ammo_Types)
			{
				if (ammo_type == t)
				{
					return true;
				}
			}
		}
		return false;
	}
}
