// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Acid Gun"), 
	Description("Performs area of denial by spraying a slow decaying acid.") 
]
public class Weapon_Acid_Gun : Projectile_Weapon
{	
	Weapon_Acid_Gun()
	{
		Name = Locale.Get("#item_acid_gun_name");

		Walk_Animation 	= "avatar_body_acid_gun";
		Fire_Animation	= "avatar_body_acid_gun_fire";

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
			Vec2(30, 57),		// S
			Vec2(40, 52),		// SE
			Vec2(47, 43),		// E
			Vec2(38, 42),		// NE
			Vec2(30, 34),		// N
			Vec2(18, 39),		// NW
			Vec2(13, 43),		// W
			Vec2(20, 52)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;
		
		Clip_Size				= 50;
		Reserve_Size			= 100;
		Initial_Ammo			= 100;
		Reload_Duration			= 2.489;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.1;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_acid_gun";
		HUD_Ammo_Icon			= "game_hud_ammoicon_acid";
		
		Fire_Loop_Sound			= "sfx_objects_weapons_acid_gun_fire";
		Reload_Sound			= "sfx_objects_weapons_acid_gun_reload";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Acid_Gun));

		// Projectile specific settings.
		Muzzle_Effect			= "acid_gun_fire";
		Is_Automatic			= true;

        // Pickup information.        
		Pickup_Sprite	= "actor_pickups_acid_gun_0";
		Pickup_Name		= Locale.Get("#item_acid_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_acid_gun_pickup";
	}
}
