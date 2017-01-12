// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Example Gun"), 
	Description("Demonstrates how to make a custom weapon!") 
]
public class Weapon_Example_Gun : Projectile_Weapon
{	
	Weapon_Example_Gun()
	{
		Name = "Example Gun";

		Walk_Animation 	= "avatar_body_example_gun";
		Fire_Animation	= "avatar_body_example_gun_fire";

		Direction_Layers = { 
			1, 	// S
			1, 	// SE
			1, 	// E
			0, 	// NE
			0, 	// N
			0, 	// NW
			1, 	// W
			1 	// SW
		};

		Muzzle_Position = {
			Vec2(29, 62),		// S
			Vec2(39, 57),		// SE
			Vec2(48, 48),		// E
			Vec2(44, 40),		// NE
			Vec2(36, 36),		// N
			Vec2(20, 40),		// NW
			Vec2(17, 48),		// W
			Vec2(25, 57)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 2.0;
		
		Clip_Size				= 99;
		Reserve_Size			= 99;
		Initial_Ammo			= 99;
		Reload_Duration			= 1.5;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.1;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_example_gun";
		HUD_Ammo_Icon			= "game_hud_ammoicon_example_gun";
		
		Fire_Sound				= "sfx_example_gun_fire";
		Reload_Sound			= "sfx_example_gun_reload";
		Dry_Fire_Sound			= "sfx_example_gun_dry_fire";
		
		AI_Priority				= 0.75;

		// Projectile specific settings.
		Muzzle_Effect			= "example_gun_fire";
		Is_Automatic			= true;
	}
}
