// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Freeze Ray"), 
	Description("Freeze everything!") 
]
public class Weapon_Freeze_Ray : Projectile_Weapon
{	
	Weapon_Freeze_Ray()
	{
		Name = Locale.Get("#item_freeze_ray_name");

		Walk_Animation 	= "avatar_body_freezeray";
		Fire_Animation	= "avatar_body_freezeray_fire";

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
			Vec2(29, 61),		// S
			Vec2(39, 58),		// SE
			Vec2(51, 51),		// E
			Vec2(48, 36),		// NE
			Vec2(29, 40),		// N
			Vec2(14, 36),		// NW
			Vec2(11, 51),		// W
			Vec2(23, 58)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 0.75;

		Screenshake_Duration	= 0.1;
		Screenshake_Intensity	= 1.0;
		
		Darken_Map				= true;
		Darken_Map_Multiplier	= 0.35f;

		Clip_Size				= 100;
		Reserve_Size			= 200;
		Initial_Ammo			= 200;
		Reload_Duration			= 2.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.05;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_freezeray";
		HUD_Ammo_Icon			= "game_hud_ammoicon_freeze";
		
		Fire_Loop_Sound			= "sfx_objects_weapons_freeze_ray_fire_loop";
		Reload_Sound			= "sfx_objects_weapons_freeze_ray_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Freeze_Ray));

		// Projectile specific settings.
		Muzzle_Effect			= "freeze_ray_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_freeze_ray_0";
		Pickup_Name		= Locale.Get("#item_freeze_ray_name");
		Pickup_Sound	= "sfx_objects_pickups_freezeray_pickup";
	}
}
