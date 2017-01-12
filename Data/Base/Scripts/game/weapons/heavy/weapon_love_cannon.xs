// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Love Cannon"), 
	Description("Heal all the things.") 
]
public class Weapon_Love_Cannon : Projectile_Weapon
{	
	Weapon_Love_Cannon()
	{
		Name = Locale.Get("#item_love_cannon_name");

		Walk_Animation 	= "avatar_body_lovecannon";
		Fire_Animation	= "avatar_body_lovecannon_fire";

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
			Vec2(24, 60),		// S
			Vec2(39, 59),		// SE
			Vec2(47, 50),		// E
			Vec2(50, 40),		// NE
			Vec2(37, 34),		// N
			Vec2(15, 40),		// NW
			Vec2(18, 51),		// W
			Vec2(26, 59)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Healing;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.0;
		Screenshake_Intensity	= 0.0;
		
		Clip_Size				= 20;
		Reserve_Size			= 0;
		Initial_Ammo			= 20;
		Reload_Duration			= 3.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.2;			
		Fire_Ammo_Usage			= 8;
		Is_Rechargable			= true;		
		Recharge_Rate			= 30.0f;
		Stamina_Based			= true;

		Hear_Radius				= 128.0f;
		
		HUD_Icon				= "game_hud_weaponicon_lovecannon";
		HUD_Ammo_Icon			= "game_hud_ammoicon_love";
		
		Fire_Sound				= "sfx_objects_weapons_love_cannon_fire";
		Reload_Sound			= "sfx_objects_weapons_love_cannon_reload";
		Dry_Fire_Sound			= "sfx_objects_weapons_shotgun_dry_fire";
		
		AI_Priority				= 0.01f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Love_Cannon));

		// Projectile specific settings.
		Muzzle_Effect			= "love_cannon_fire";
		Is_Automatic			= true;
        
		Pickup_Sprite	= "actor_pickups_lovecannon_0";
		Pickup_Name		= Locale.Get("#item_love_cannon_name");
		Pickup_Sound	= "sfx_objects_pickups_lovecannon_pickup";
	}
}
