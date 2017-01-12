 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Weapon Exploder Zombie"), 
	Description("Claws for the exploder zombie.") 
]
public class Weapon_Exploder_Zombie : Projectile_Weapon
{		
	Weapon_Exploder_Zombie()
	{
		Name = "<internal_weapon>";

		Walk_Animation 	= "";
		Fire_Animation	= "enemy_exploder_zombie_charge";

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
			Vec2(24, 24),		// S
			Vec2(24, 24),		// SE
			Vec2(24, 24),		// E
			Vec2(24, 24),		// NE
			Vec2(24, 24),		// N
			Vec2(24, 24),		// NW
			Vec2(24, 24),		// W
			Vec2(24, 24)		// SW
		};  

		Slot					= Weapon_Slot.Melee;
		SubType					= Weapon_SubType.Enemy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;
		
		Clip_Size				= 100;
		Reserve_Size			= 0;
		Initial_Ammo			= 100;
		Reload_Duration			= 0.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.5f;	
		Fire_Ammo_Usage			= 30;	
		Is_Rechargable			= true;			
		Recharge_Rate			= 50.0f;
		
		HUD_Icon				= "game_hud_weaponicon_explosive";
		HUD_Ammo_Icon			= "game_hud_ammoicon_explosive";
		
		Fire_Sound				= "sfx_objects_enemies_zombie_slash";
		Reload_Sound			= "";
		
		AI_Priority				= 0.1f;

		Item_Type 				= null;

		// Projectile specific settings.
		Muzzle_Effect			= "exploder_zombie_slash";
		Is_Automatic			= false;
	}
}
