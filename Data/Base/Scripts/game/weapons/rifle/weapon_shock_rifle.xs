// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.beam_weapon;

[
	Name("Shock Rifle"), 
	Description("Zaaaap") 
]
public class Weapon_Shock_Rifle : Beam_Weapon
{	
	Weapon_Shock_Rifle()
	{
		Name = Locale.Get("#item_shock_rifle_name");

		Walk_Animation 	= "avatar_body_shock_rifle";	
		Fire_Animation	= "avatar_body_shock_rifle_fire";

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
			Vec2(29, 64),		// S
			Vec2(42, 59),		// SE
			Vec2(50, 49),		// E
			Vec2(46, 38),		// NE
			Vec2(36, 36),		// N
			Vec2(18, 38),		// NW
			Vec2(15, 48),		// W
			Vec2(22, 59)		// SW
		};  
		
		Slot					= Weapon_Slot.Rifle;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 0.75;
		
		Screenshake_Duration	= 1.5;
		Screenshake_Intensity	= 0.05;
		
		Clip_Size				= 100;
		Reserve_Size			= 100;
		Initial_Ammo			= 100;
		Reload_Duration			= 2.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.450;			
		Fire_Ammo_Usage			= 100;
		Is_Rechargable			= true;
		Recharge_Rate			= 15.0f;	

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_shock_rifle";
		HUD_Ammo_Icon			= "game_hud_ammoicon_shock_rifle";
		
		Fire_Sound				= "sfx_objects_weapons_shock_rifle_fire";
		Reload_Sound			= "sfx_objects_weapons_shock_rifle_reload";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Shock_Rifle));

		// Projectile specific settings.
		Beam_Effect				= "shock_rifle_fire";
		Charge_Time				= 0.0f;
		Beam_Duration			= 3.6f;
		Lock_Direction			= true;
        
		Pickup_Sprite	= "actor_pickups_shock_rifle_0";
		Pickup_Name		= Locale.Get("#item_shock_rifle_name");
		Pickup_Sound	= "sfx_objects_pickups_shockrifle_pickup";
	}
}
