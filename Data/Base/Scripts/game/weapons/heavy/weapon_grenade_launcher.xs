// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Grenade Launchers"), 
	Description("Need moar 'nades.") 
]
public class Weapon_Grenade_Launcher : Projectile_Weapon
{	
	Weapon_Grenade_Launcher()
	{
		Name = Locale.Get("#item_grenade_launcher_name");

		Walk_Animation 	= "avatar_body_grenadelauncher";
		Fire_Animation	= "avatar_body_grenadelauncher_fire";

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
			Vec2(24, 58),		// S
			Vec2(38, 56),		// SE
			Vec2(45, 48),		// E
			Vec2(44, 39),		// NE
			Vec2(36, 35),		// N
			Vec2(18, 39),		// NW
			Vec2(19, 48),		// W
			Vec2(26, 56)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.6;
		Screenshake_Intensity	= 1;
		
		Clip_Size				= 8;
		Reserve_Size			= 24;
		Initial_Ammo			= 24;
		Reload_Duration			= 1.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.45;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_grenade_launcher";
		HUD_Ammo_Icon			= "game_hud_ammoicon_grenade";
		
		Fire_Sound				= "sfx_objects_weapons_grenade_launcher_fire";
		Reload_Sound			= "sfx_objects_weapons_grenade_launcher_reload";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Grenade_Launcher));

		// Projectile specific settings.
		Muzzle_Effect			= "grenade_launcher_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_grenade_launcher_0";
		Pickup_Name		= Locale.Get("#item_grenade_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_grenade_launcher_pickup";
	}
}
