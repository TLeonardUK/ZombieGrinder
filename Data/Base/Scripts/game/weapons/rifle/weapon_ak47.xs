// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Name("AK47"), 
	Description("Most overrated weapon ever.") 
]
public class Weapon_Ak47 : Projectile_Weapon
{	
	Weapon_Ak47()
	{
		Name = Locale.Get("#item_ak47_name");

		Walk_Animation 	= "avatar_body_ak47";
		Fire_Animation	= "avatar_body_ak47_fire";

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
			Vec2(29, 58),		// S
			Vec2(40, 57),		// SE
			Vec2(51, 46),		// E
			Vec2(47, 37),		// NE
			Vec2(34, 32),		// N
			Vec2(17, 37),		// NW
			Vec2(14, 46),		// W
			Vec2(23, 57)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Rifle;
		SubType					= Weapon_SubType.Rifle;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 2.0;
		
		Clip_Size				= 25;
		Reserve_Size			= 300;
		Initial_Ammo			= 300;
		Reload_Duration			= 1.187;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.115;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_ak47";
		HUD_Ammo_Icon			= "game_hud_ammoicon_ak47";
		
		Fire_Sound				= "sfx_objects_weapons_ak47_fire";
		Reload_Sound			= "sfx_objects_weapons_ak47_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 2.0f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Ak47));

		// Projectile specific settings.
		Muzzle_Effect			= "ak47_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_ak47_0";
		Pickup_Name		= Locale.Get("#item_ak47_name");
		Pickup_Sound	= "sfx_objects_pickups_ak47_pickup";
	}
}
