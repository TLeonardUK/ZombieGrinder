// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Name("Winchester Rifle"), 
	Description("Wait for all this to blow over.") 
]
public class Weapon_Winchester_Rifle : Projectile_Weapon
{	
	Weapon_Winchester_Rifle()
	{
		Name = Locale.Get("#item_winchester_rifle_name");

		Walk_Animation 	= "avatar_body_winchester_rifle";
		Fire_Animation	= "avatar_body_winchester_rifle_fire";

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
			Vec2(30, 63),		// S
			Vec2(41, 58),		// SE
			Vec2(52, 48),		// E
			Vec2(47, 37),		// NE
			Vec2(36, 36),		// N
			Vec2(17, 37),		// NW
			Vec2(13, 48),		// W
			Vec2(22, 59)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Rifle;
		SubType					= Weapon_SubType.Rifle;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.9;
		Screenshake_Intensity	= 4.0;
		
		Clip_Size				= 1;
		Reserve_Size			= 48;
		Initial_Ammo			= 48;
		Reload_Duration			= 0.673;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.10;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_winchester_rifle";
		HUD_Ammo_Icon			= "game_hud_ammoicon_winchester_rifle";
		
		Fire_Sound				= "sfx_objects_weapons_winchester_rifle_fire";
		Reload_Sound			= "sfx_objects_weapons_winchester_rifle_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 3.0f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Winchester_Rifle));

		// Projectile specific settings.
		Muzzle_Effect			= "winchester_rifle_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_winchester_rifle_0";
		Pickup_Name		= Locale.Get("#item_winchester_rifle_name");
		Pickup_Sound	= "sfx_objects_pickups_winchester_rifle_pickup";
	}
}
