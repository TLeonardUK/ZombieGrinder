// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Bolt Gun"), 
	Description("Zaps enemies and stops them moving.") 
]
public class Weapon_Bolt_Gun : Projectile_Weapon
{	
	Weapon_Bolt_Gun()
	{
		Name = Locale.Get("#item_bolt_gun_name");

		Walk_Animation 	= "avatar_body_bolt_gun";
		Fire_Animation	= "avatar_body_bolt_gun_fire";

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
			Vec2(29, 62),		// S
			Vec2(39, 57),		// SE
			Vec2(48, 48),		// E
			Vec2(44, 40),		// NE
			Vec2(34, 36),		// N
			Vec2(20, 40),		// NW
			Vec2(17, 48),		// W
			Vec2(25, 57)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 2.0;
		
		Clip_Size				= 4;
		Reserve_Size			= 50;
		Initial_Ammo			= 50;
		Reload_Duration			= 1.5;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.3;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_bolt_gun";
		HUD_Ammo_Icon			= "game_hud_ammoicon_bolt_gun";
		
		Fire_Sound				= "sfx_objects_weapons_bolt_gun_fire";
		Reload_Sound			= "sfx_objects_weapons_bolt_gun_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 0.75;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Bolt_Gun));

		// Projectile specific settings.
		Muzzle_Effect			= "bolt_gun_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_bolt_gun_0";
		Pickup_Name		= Locale.Get("#item_bolt_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_boltgun_pickup";
	}
}
