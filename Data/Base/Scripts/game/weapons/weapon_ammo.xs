// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log; 
using game.profile.profile;

[
	Name("Weapon Ammo"), 
	Description("Base class for all weapon ammo modifiers.") 
]
public class Weapon_Ammo
{	
	// Display name for weapon ammo.
	public string Name;

	// Muzzle flash effect modifier for this weapon.
	public string Effect_Modifier = "";

	// Clip size modifier.
	public float Clip_Size_Scalar = 1.0f;

	// HUD Information.
	public string HUD_Ammo_Icon = "";
}
 