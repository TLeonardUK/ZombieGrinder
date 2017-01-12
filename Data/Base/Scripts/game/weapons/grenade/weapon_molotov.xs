// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Molotov"), 
	Description("AOE grenade that burns users.") 
]
public class Weapon_Molotov : Projectile_Weapon
{	
	Weapon_Molotov()
	{
		Name = Locale.Get("#item_molotov_name");

		Walk_Animation 	= "avatar_body_molotov";
		Fire_Animation	= "avatar_body_molotov_fire";

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
			Vec2(29, 47),		// S
			Vec2(24, 47),		// SE
			Vec2(33, 47),		// E
			Vec2(33, 46),		// NE
			Vec2(33, 43),		// N
			Vec2(23, 46),		// NW
			Vec2(27, 47),		// W
			Vec2(32, 47)		// SW
		};  

		Slot					= Weapon_Slot.Grenade;
		SubType					= Weapon_SubType.Grenade;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;
		
		Clip_Size				= 1;
		Reserve_Size			= 16;
		Initial_Ammo			= 16;
		Reload_Duration			= 0.500;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.500;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Can_Buffs_Regenerate_Ammo	= false;
		Ignore_Spawn_Collision		= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_molotov";
		HUD_Ammo_Icon			= "game_hud_ammoicon_molotov";
		
		Fire_Sound				= "sfx_objects_weapons_molotov_fire";
		
		AI_Priority				= 0.1f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Molotov));

		// Projectile specific settings.
		Muzzle_Effect			= "molotov_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_molotov_0";
		Pickup_Name		= Locale.Get("#item_molotov_name");
		Pickup_Sound	= "sfx_objects_pickups_molotov_pickup";
	}	
}
