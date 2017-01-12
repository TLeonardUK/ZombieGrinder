// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using system.actors.actor;
using system.fx;
using runtime.log;

[
	Name("Muzzle Flash"), 
	Description("Shows a muzzle flash effect and tracks it's parent weapon. When parent is destroyed the muzzle flash will be destroyed as well.") 
]
public class Muzzle_Flash : Actor
{
	// Weapon type this muzzle flash is attached to.
	public Weapon Attached_Weapon;

	// Effect being shown!
	public string Effect_Name;
	public Effect Flash_Effect;

	// Ammo modifier for this flash.
	public string Effect_Modifier;

	// Will cause the particles emitter by this effect to ingore any collision they are spawned within.
	public bool Ignore_Spawn_Collision;

    // Offset in degrees for the direction, used to do multibarrel effects.
    public float Direction_Offset;

	// Constructor.
	Muzzle_Flash()
	{
		Tick_Priority = Tick_Priority_Type.Late;
	}

	// Despawns this actor and its effect.
	public void Dispose()
	{
		if (Flash_Effect != null && Scene.Is_Spawned(Flash_Effect))
		{
			Scene.Despawn(Flash_Effect);
		}
		Scene.Despawn(this);
	}
    
	// Returns true when flash is finished.
	public bool Is_Active()
	{
        return (Flash_Effect != null && Scene.Is_Spawned(Flash_Effect));
	}

	// Makes the particles thie effect is spawning invisible.
	public void Hide()
	{
		if (Flash_Effect != null && Scene.Is_Spawned(Flash_Effect))
		{
			Flash_Effect.Get_Component().Visible = false;
		}
	}

	// Default state, just tracks owner and waits for it to expire.
	default state Running
	{
		event void On_Tick()
		{
            bool bValid = false;

			if (Attached_Weapon.Owner != null && Scene.Is_Spawned(Attached_Weapon.Owner))
			{
                Pawn pwn = <Pawn>Attached_Weapon.Owner;
                if (pwn == null || !pwn.Is_Incapacitated())
                {
                    bValid = true;

				    Vec3 muzzle_position = Attached_Weapon.Owner.Position + Vec3(Attached_Weapon.Muzzle_Position[Attached_Weapon.Owner.Get_Direction()], 0.0f);
				    float muzzle_angle = Direction_Helper.Angles[Attached_Weapon.Owner.Get_Direction()] + Direction_Offset;
    
				    if (Flash_Effect != null)
				    {
					    if (!Scene.Is_Spawned(Flash_Effect))
					    {
						    Dispose();
					    }
					    else
					    {					
						    Flash_Effect.Position = muzzle_position;
						    Flash_Effect.Rotation = muzzle_angle;
					    }
				    }
				    else
				    {
					    Flash_Effect = FX.Spawn(Effect_Name, muzzle_position, muzzle_angle, Attached_Weapon.Owner.Layer, Parent, 0, Effect_Modifier, Attached_Weapon.SubType, Attached_Weapon.Item_Type, Attached_Weapon.Upgrade_Tree_Modifiers);
					    Flash_Effect.Get_Component().Ignore_Spawn_Collision = Ignore_Spawn_Collision;
					    Flash_Effect.DisposeOnFinish = true;
				    }
                }
			}

            if (!bValid)
            {            
				Dispose();
            }
		}
	}
}