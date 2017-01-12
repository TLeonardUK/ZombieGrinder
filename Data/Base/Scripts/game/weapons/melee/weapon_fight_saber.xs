// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Fight Sabar"), 
	Description("Ziiiiiiing") 
]
public class Weapon_Fight_Saber : Projectile_Weapon
{	
	Weapon_Fight_Saber()
	{
		Name = Locale.Get("#item_fight_saber_name");

		Walk_Animation 	= "avatar_body_fight_saber";
		Fire_Animation	= "avatar_body_fight_saber_fire";

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
			Vec2(30, 66),		// S
			Vec2(49, 62),		// SE
			Vec2(57, 48),		// E
			Vec2(46, 37),		// NE
			Vec2(28, 20),		// N
			Vec2(16, 37),		// NW
			Vec2(7, 48),		// W
			Vec2(14, 62)		// SW
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
		
		HUD_Icon				= "game_hud_weaponicon_fight_saber";
		HUD_Ammo_Icon			= "game_hud_ammoicon_melee";
		
		Fire_Sound				= "sfx_objects_weapons_fight_saber_no_hit";
		Reload_Sound			= "";
		
		AI_Priority				= 0.75f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Fight_Saber));

		// Projectile specific settings.
		Muzzle_Effect			= "fight_saber_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_fight_saber_0";
		Pickup_Name		= Locale.Get("#item_fight_saber_name");
		Pickup_Sound	= "sfx_objects_pickups_melee_pickup";
	}
}
