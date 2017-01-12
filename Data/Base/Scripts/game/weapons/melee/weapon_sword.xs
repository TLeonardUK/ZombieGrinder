// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Sword"), 
	Description("Ziiiiiiing") 
]
public class Weapon_Sword : Projectile_Weapon
{	
	Weapon_Sword()
	{
		Name = Locale.Get("#item_sword_name");

		Walk_Animation 	= "avatar_body_sword";
		Fire_Animation	= "avatar_body_sword_fire";

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
			Vec2(30, 68),		// S
			Vec2(51, 64),		// SE
			Vec2(59, 48),		// E
			Vec2(48, 35),		// NE
			Vec2(28, 18),		// N
			Vec2(14, 35),		// NW
			Vec2(5, 48),		// W
			Vec2(12, 64)		// SW
		};  

		Slot					= Weapon_Slot.Melee;
		SubType					= Weapon_SubType.Melee;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;

		Is_Tintable				= true;
		
		Clip_Size				= 100;
		Reserve_Size			= 0;
		Initial_Ammo			= 100;
		Reload_Duration			= 2.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.2f;	
		Fire_Ammo_Usage			= 25;	
		Is_Rechargable			= true;		
		Recharge_Rate			= 30.0f;
		Stamina_Based			= true;
		
		HUD_Icon				= "game_hud_weaponicon_sword";
		HUD_Ammo_Icon			= "game_hud_ammoicon_melee";
		
		Fire_Sound				= "sfx_objects_weapons_sword_no_hit";
		Reload_Sound			= "";
		
		AI_Priority				= 0.75f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Sword));

		// Projectile specific settings.
		Muzzle_Effect			= "sword_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_sword_0";
		Pickup_Name		= Locale.Get("#item_sword_name");
		Pickup_Sound	= "sfx_objects_pickups_melee_pickup";
	}
}
