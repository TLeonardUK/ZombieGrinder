 // -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Flare Gun"), 
	Description("Fizzle") 
]
public class Weapon_Flare_Gun : Projectile_Weapon
{	
	Weapon_Flare_Gun()
	{
		Name = Locale.Get("#item_flare_gun_name");

		Walk_Animation 	= "avatar_body_flaregun";
		Fire_Animation	= "avatar_body_flaregun_fire";

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
			Vec2(31, 63),		// S
			Vec2(42, 58),		// SE
			Vec2(47, 45),		// E
			Vec2(46, 37),		// NE
			Vec2(28, 40),		// N
			Vec2(16, 37),		// NW
			Vec2(17, 45),		// W
			Vec2(20, 58)		// SW
		};  

		Slot					= Weapon_Slot.Pistol;
		SubType					= Weapon_SubType.Small_Arms;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0.3;
		Screenshake_Intensity	= 1;
		
		Clip_Size				= 1;
		Reserve_Size			= 40;
		Initial_Ammo			= 40;
		Reload_Duration			= 1.265;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.05;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_flaregun";
		HUD_Ammo_Icon			= "game_hud_ammoicon_flare";
		
		Fire_Sound				= "sfx_objects_weapons_flare_gun_fire";
		Reload_Sound			= "sfx_objects_weapons_flare_gun_reload";
		
		AI_Priority				= 1.0f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Flare_Gun));

		// Projectile specific settings.
		Muzzle_Effect			= "flare_gun_fire";
		Is_Automatic			= false;

		Pickup_Sprite	= "actor_pickups_flaregun_0";
		Pickup_Name		= Locale.Get("#item_flare_gun_name");
		Pickup_Sound	= "sfx_objects_pickups_flaregun_pickup";
	}
}
