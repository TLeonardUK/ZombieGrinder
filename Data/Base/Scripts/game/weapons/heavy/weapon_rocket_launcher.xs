// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Rocket Launcher"), 
	Description("BOOOOOOOOME") 
]
public class Weapon_Rocket_Launcher : Projectile_Weapon
{	
	Weapon_Rocket_Launcher()
	{
		Name = Locale.Get("#item_rocket_launcher_name");

		Walk_Animation 	= "avatar_body_rocketlauncher";		
		Fire_Animation	= "avatar_body_rocketlauncher_fire";

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
			Vec2(23, 51),		// S
			Vec2(29, 50),		// SE
			Vec2(44, 44),		// E
			Vec2(46, 36),		// NE
			Vec2(39, 30),		// N
			Vec2(27, 32),		// NW
			Vec2(19, 41),		// W
			Vec2(21, 47)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 0.75;

		Screenshake_Duration	= 1.5;
		Screenshake_Intensity	= 5;
		
		Clip_Size				= 1;
		Reserve_Size			= 10;
		Initial_Ammo			= 10;
		Reload_Duration			= 2.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.450;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_rocket_launcher";
		HUD_Ammo_Icon			= "game_hud_ammoicon_rocket";
		
		Fire_Sound				= "sfx_objects_weapons_rocket_launcher_fire";
		Reload_Sound			= "sfx_objects_weapons_rocket_launcher_reload";
		
		AI_Priority				= 0.75f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Rocket_Launcher));

		// Projectile specific settings.
		Muzzle_Effect			= "rocket_launcher_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_rocket_launcher_0";
		Pickup_Name		= Locale.Get("#item_rocket_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_rocket_launcher_pickup";
	}
}
