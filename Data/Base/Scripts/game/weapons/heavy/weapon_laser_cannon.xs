// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.beam_weapon;

[
	Name("Laser Cannon"), 
	Description("OP plz nerf.") 
]
public class Weapon_Laser_Cannon : Beam_Weapon
{	
	Weapon_Laser_Cannon()
	{
		Name = Locale.Get("#item_laser_cannon_name");

		Walk_Animation 	= "avatar_body_lasercannon";	
		Fire_Animation	= "avatar_body_lasercannon_fire";

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
			Vec2(23, 53),		// S
			Vec2(32, 51),		// SE
			Vec2(49, 45),		// E
			Vec2(47, 37),		// NE
			Vec2(39, 30),		// N
			Vec2(17, 36),		// NW
			Vec2(15, 45),		// W
			Vec2(32, 51)		// SW
		};  

		Slot					= Weapon_Slot.Heavy;
		SubType					= Weapon_SubType.Heavy;
		Speed_Multiplier		= 0.75;

		Screenshake_Duration	= 2.0;
		Screenshake_Intensity	= 0.15;
		
		Clip_Size				= 1;
		Reserve_Size			= 10;
		Initial_Ammo			= 10;
		Reload_Duration			= 4.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.450;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Chromatic_Arboration_Intensity	= 4.5f;
		
		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_lasercannon";
		HUD_Ammo_Icon			= "game_hud_ammoicon_laser";
		
		Fire_Sound				= "sfx_objects_weapons_laser_cannon_fire";
		Reload_Sound			= "sfx_objects_weapons_laser_cannon_reload";
		Charge_Sound			= "sfx_objects_weapons_laser_cannon_charge";
		
		AI_Priority				= 0.5f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Laser_Cannon));

		// Projectile specific settings.
		Charge_Effect			= "laser_cannon_charge";
		Beam_Effect				= "laser_cannon_fire";
		Charge_Time				= 2.31f;
		Beam_Duration			= 4.97f;
		Lock_Direction			= true;
        
		Pickup_Sprite	= "actor_pickups_lasercannon_0";
		Pickup_Name		= Locale.Get("#item_laser_cannon_name");
		Pickup_Sound	= "sfx_objects_pickups_lasercannon_pickup";
	}
}
 