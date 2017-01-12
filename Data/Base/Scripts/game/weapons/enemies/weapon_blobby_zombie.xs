 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Weapon Blobby Zombie"), 
	Description("Claws for the blobby zombie.") 
]
public class Weapon_Blobby_Zombie : Projectile_Weapon
{		
	Weapon_Blobby_Zombie()
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
		Fire_Interval			= 0.5f;	
		Fire_Ammo_Usage			= 40;	
		Is_Rechargable			= true;			
		Recharge_Rate			= 15.0f;
		Stamina_Based			= true;

		HUD_Icon				= "game_hud_weaponicon_enemy";
		HUD_Ammo_Icon			= "game_hud_ammoicon_enemy";
		
		Fire_Sound				= "sfx_objects_enemies_bouncer_squash_1";
		Reload_Sound			= "";
		
		AI_Priority				= 0.1f;
		
		Item_Type 				= null;

		// Projectile specific settings.
		Muzzle_Effect			= "blobby_zombie_puff";
		Is_Automatic			= false;
	}
}
