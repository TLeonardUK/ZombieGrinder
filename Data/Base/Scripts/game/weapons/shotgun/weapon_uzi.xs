// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Name("Uzi"), 
	Description("Such bullets, much wow.") 
]
public class Weapon_Uzi : Projectile_Weapon
{		
	Weapon_Uzi()
	{
		Name = Locale.Get("#item_uzi_name");

		Walk_Animation 	= "avatar_body_uzi";
		Fire_Animation	= "avatar_body_uzi_fire";

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
			Vec2(30, 54),		// S
			Vec2(40, 50),		// SE
			Vec2(45, 45),		// E
			Vec2(46, 36),		// NE
			Vec2(30, 34),		// N
			Vec2(17, 36),		// NW
			Vec2(18, 45),		// W
			Vec2(23, 50)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Rifle;
		SubType					= Weapon_SubType.Small_Arms;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 2.0;
		
		Clip_Size				= 35;
		Reserve_Size			= 200;
		Initial_Ammo			= 200;
		Reload_Duration			= 1.187;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.075;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_uzi";
		HUD_Ammo_Icon			= "game_hud_ammoicon_smg";
		
		Fire_Sound				= "sfx_objects_weapons_uzi_fire";
		Reload_Sound			= "sfx_objects_weapons_uzi_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";

		AI_Priority				= 1.0f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Uzi));

		// Projectile specific settings.
		Muzzle_Effect			= "uzi_fire";
		Is_Automatic			= true;

		Pickup_Sprite	= "actor_pickups_uzi_0";
		Pickup_Name		= Locale.Get("#item_uzi_name");
		Pickup_Sound	= "sfx_objects_pickups_uzi_pickup";
	}
}
