// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Sticky Launcher"), 
	Description("Sprays remote-detonated gel.") 
]
public class Weapon_Sticky_Launcher : Projectile_Weapon
{	
	Weapon_Sticky_Launcher()
	{
		Name = Locale.Get("#item_sticky_launcher_name");

		Walk_Animation 	= "avatar_body_stickylauncher";
		Fire_Animation	= "avatar_body_stickylauncher_fire";

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
			Vec2(39, 61),		// S
			Vec2(38, 57),		// SE
			Vec2(47, 48),		// E
			Vec2(48, 38),		// NE
			Vec2(24, 35),		// N
			Vec2(17, 38),		// NW
			Vec2(17, 48),		// W
			Vec2(28, 56)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;
		
		Clip_Size				= 50;
		Reserve_Size			= 100;
		Initial_Ammo			= 100;
		Reload_Duration			= 2.489;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.05;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Can_Alt_Fire			= true;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_sticky_launcher";
		HUD_Ammo_Icon			= "game_hud_ammoicon_sticky";
		
		Fire_Loop_Sound			= "sfx_objects_weapons_sticky_launcher_fire";
		Reload_Sound			= "sfx_objects_weapons_sticky_launcher_reload";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Sticky_Launcher));

		// Projectile specific settings.
		Muzzle_Effect			= "sticky_launcher_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_sticky_launcher_0";
		Pickup_Name		= Locale.Get("#item_sticky_launcher_name");
		Pickup_Sound	= "sfx_objects_pickups_sticky_launcher_pickup";
	}
}
