// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Love Thrower"), 
	Description("Heal all the things, with AOE!") 
]
public class Weapon_Love_Thrower : Projectile_Weapon
{	
	Weapon_Love_Thrower()
	{
		Name = Locale.Get("#item_love_thrower_name");

		Walk_Animation 	= "avatar_body_love_thrower";
		Fire_Animation	= "avatar_body_love_thrower_fire";

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
			Vec2(30, 58),		// S
			Vec2(37, 56),		// SE
			Vec2(48, 53),		// E
			Vec2(49, 42),		// NE
			Vec2(36, 38),		// N
			Vec2(20, 41),		// NW
			Vec2(17, 53),		// W
			Vec2(30, 59)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Healing;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.0;
		Screenshake_Intensity	= 0.0;
		
		Darken_Map				= true;
		Darken_Map_Multiplier	= 0.35f;

		Clip_Size				= 20;
		Reserve_Size			= 0;
		Initial_Ammo			= 100;
		Reload_Duration			= 1.5;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.05;			
		Fire_Ammo_Usage			= 3;
		Is_Rechargable			= true;
		Use_Individual_Reload	= false;
		Recharge_Rate			= 30.0f;
		Stamina_Based			= true;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_lovethrower";
		HUD_Ammo_Icon			= "game_hud_ammoicon_love";
		
		Fire_Loop_Sound			= "sfx_objects_weapons_love_thrower_fire";
		Reload_Sound			= "sfx_objects_weapons_love_thrower_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 0.01f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Love_Thrower));

		// Projectile specific settings.
		Muzzle_Effect			= "love_thrower_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_lovethrower_0";
		Pickup_Name		= Locale.Get("#item_love_thrower_name");
		Pickup_Sound	= "sfx_objects_pickups_lovethrower_pickup";
	}
}
