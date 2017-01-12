// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Flamethrower"), 
	Description("Torch everything!") 
]
public class Weapon_Flamethrower : Projectile_Weapon
{	
	Weapon_Flamethrower()
	{
		Name = Locale.Get("#item_flamethrower_name");

		Walk_Animation 	= "avatar_body_flamethrower";
		Fire_Animation	= "avatar_body_flamethrower_fire";

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
		Speed_Multiplier		= 0.5;

		Screenshake_Duration	= 0.0;
		Screenshake_Intensity	= 0.0;
		
		Darken_Map				= true;
		Darken_Map_Multiplier	= 0.35f;

		Clip_Size				= 100;
		Reserve_Size			= 500;
		Initial_Ammo			= 500;
		Reload_Duration			= 2.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.035;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_flamethrower";
		HUD_Ammo_Icon			= "game_hud_ammoicon_incendiary";
		
		Fire_Loop_Start_Sound	= "sfx_objects_weapons_flamethrower_fire_start";
		Fire_Loop_Sound			= "sfx_objects_weapons_flamethrower_fire_loop";
		Reload_Sound			= "sfx_objects_weapons_flamethrower_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 1.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Flamethrower));

		// Projectile specific settings.
		Muzzle_Effect			= "flamethrower_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_flamethrower_0";
		Pickup_Name		= Locale.Get("#item_flamethrower_name");
		Pickup_Sound	= "sfx_objects_pickups_flamethrower_pickup";
	}
}
