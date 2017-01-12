// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;

[
	Name("Developer Pistol"), 
	Description("You can run but you can't hide!") 
]
public class Weapon_Developer_Pistol : Projectile_Weapon
{	
	Weapon_Developer_Pistol()
	{
		Name = Locale.Get("#item_developer_pistol_name");

		Walk_Animation 	= "avatar_body_pistol";
		Fire_Animation	= "avatar_body_pistol_fire";

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
			Vec2(31, 58),		// S
			Vec2(41, 55),		// SE
			Vec2(47, 45),		// E
			Vec2(46, 37),		// NE
			Vec2(31, 34),		// N
			Vec2(17, 37),		// NW
			Vec2(16, 45),		// W
			Vec2(22, 53)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Pistol;
		SubType					= Weapon_SubType.Small_Arms;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.3;
		Screenshake_Intensity	= 1;
		
		Clip_Size				= 999;
		Reserve_Size			= 0;
		Initial_Ammo			= 99;
		Reload_Duration			= 1.0;
		Has_Unlimited_Ammo	 	= true;
		Fire_Interval			= 0.05;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_pistol";
		HUD_Ammo_Icon			= "game_hud_ammoicon_pistol";
		
		Fire_Sound				= "sfx_objects_weapons_pistol_fire";
		Reload_Sound			= "sfx_objects_weapons_pistol_reload";
		
		AI_Priority				= 0.010f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Developer_Pistol));

		// Projectile specific settings.
		Muzzle_Effect			= "developer_pistol_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_pistol_0";
		Pickup_Name		= Locale.Get("#item_developer_pistol_name");
		Pickup_Sound	= "sfx_objects_pickups_pistol_pickup";
	}
}
