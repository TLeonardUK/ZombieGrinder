 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Weapon Bouncer Zombie"), 
	Description("Claws for the bouncer zombie.") 
]
public class Weapon_Bouncer_Zombie : Projectile_Weapon
{		
	Weapon_Bouncer_Zombie()
	{
		Name = "<internal_weapon>";

		Walk_Animation 	= "";
		Fire_Animation	= "";

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
			Vec2(14, 18),		// S
			Vec2(14, 18),		// SE
			Vec2(14, 18),		// E
			Vec2(14, 18),		// NE
			Vec2(14, 18),		// N
			Vec2(14, 18),		// NW
			Vec2(14, 18),		// W
			Vec2(14, 18)		// SW
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
		Fire_Interval			= 1.5f;	
		Fire_Ammo_Usage			= 30;	
		Is_Rechargable			= true;			
		Recharge_Rate			= 50.0f;
		
		HUD_Icon				= "game_hud_weaponicon_enemy";
		HUD_Ammo_Icon			= "game_hud_ammoicon_enemy";
		
		Fire_Sound				= "sfx_objects_enemies_bouncer_squash_1";
		Reload_Sound			= "";
		
		AI_Priority				= 0.1f;

		Item_Type 				= null;
		
		// Projectile specific settings.
		Muzzle_Effect			= "bouncer_zombie_fire";
		Is_Automatic			= false;
	}
}
