// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Rocket Propelled Chainsaw"), 
	Description("Seriously?") 
]
public class Weapon_RPC : Projectile_Weapon
{	
	Weapon_RPC()
	{
		Name = Locale.Get("#item_rpc_name");

		Walk_Animation 	= "avatar_body_rpc";
		Fire_Animation	= "avatar_body_rpc_fire";

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
			Vec2(23, 51),		// S
			Vec2(29, 50),		// SE
			Vec2(44, 44),		// E
			Vec2(46, 36),		// NE
			Vec2(39, 30),		// N
			Vec2(27, 32),		// NW
			Vec2(19, 41),		// W
			Vec2(21, 47)		// SW
		};  
		
		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 0.75;

		Screenshake_Duration	= 0.5;
		Screenshake_Intensity	= 2;
		
		Clip_Size				= 1;
		Reserve_Size			= 10;
		Initial_Ammo			= 10;
		Reload_Duration			= 3.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.450;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_rpc";
		HUD_Ammo_Icon			= "game_hud_ammoicon_rocket";
		
		Fire_Sound				= "sfx_objects_weapons_rpc_fire";
		Reload_Sound			= "sfx_objects_weapons_rpc_reload";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Rpc));

		// Projectile specific settings.
		Muzzle_Effect			= "rpc_fire";
		Is_Automatic			= false;
        
		Pickup_Sprite	= "actor_pickups_rpc_0";
		Pickup_Name		= Locale.Get("#item_rpc_name");
		Pickup_Sound	= "sfx_objects_pickups_rpc_pickup";
	}
}
