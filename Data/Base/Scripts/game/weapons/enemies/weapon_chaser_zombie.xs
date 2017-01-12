 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Weapon Chaser Zombie"), 
	Description("Claws for the chaser zombie.") 
]
public class Weapon_Chaser_Zombie : Projectile_Weapon
{		
	Weapon_Chaser_Zombie()
	{
		Name = "<internal_weapon>";

		Walk_Animation 	= "";
		Fire_Animation	= "enemy_chaser_zombie_attack";

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
			Vec2(17, 33),		// S
			Vec2(28, 30),		// SE
			Vec2(30, 24),		// E
			Vec2(26, 13),		// NE
			Vec2(16, 8),		// N
			Vec2(9, 13),		// NW
			Vec2(3, 24),		// W
			Vec2(5, 30)			// SW
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
		Stamina_Based			= true;

		HUD_Icon				= "game_hud_weaponicon_enemy";
		HUD_Ammo_Icon			= "game_hud_ammoicon_enemy";
		
		Fire_Sound				= "sfx_objects_enemies_zombie_slash_1,sfx_objects_enemies_zombie_slash_2,sfx_objects_enemies_zombie_slash_3,sfx_objects_enemies_zombie_slash_4";
		Reload_Sound			= "";
		
		AI_Priority				= 0.1f;

		Item_Type 				= null;

		// Projectile specific settings.
		Muzzle_Effect			= "chaser_zombie_slash";
		Is_Automatic			= false;
	}
}
