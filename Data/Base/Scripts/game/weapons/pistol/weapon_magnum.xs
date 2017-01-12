// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Name("Magnum"), 
	Description("Maximum penetration!") 
]
public class Weapon_Magnum : Projectile_Weapon
{	
	Weapon_Magnum()
	{
		Name = Locale.Get("#item_magnum_name");

		Walk_Animation 	= "avatar_body_magnum";
		Fire_Animation	= "avatar_body_magnum_fire";

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
			Vec2(30, 59),		// S
			Vec2(43, 55),		// SE
			Vec2(54, 44),		// E
			Vec2(48, 30),		// NE
			Vec2(32, 28),		// N
			Vec2(14, 30),		// NW
			Vec2(6,  44),		// W
			Vec2(19, 55)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Pistol;
		SubType					= Weapon_SubType.Small_Arms;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 4;
		
		Clip_Size				= 6;
		Reserve_Size			= 32;
		Initial_Ammo			= 32;
		Reload_Duration			= 2.489;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.3;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= true;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_magnum";
		HUD_Ammo_Icon			= "game_hud_ammoicon_magnum";
		
		Fire_Sound				= "sfx_objects_weapons_magnum_fire";
		Reload_Individual_Sound	= "sfx_objects_weapons_magnum_reload";
		Reload_Finish_Sound		= "sfx_objects_pickups_magnum_reload_end";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 2.0f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Magnum));

		// Projectile specific settings.
		Muzzle_Effect			= "magnum_fire";
		Is_Automatic			= false;

		Pickup_Sprite	= "actor_pickups_magnum_0";
		Pickup_Name		= Locale.Get("#item_magnum_name");
		Pickup_Sound	= "sfx_objects_pickups_magnum_pickup";
	}
}
