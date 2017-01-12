// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.placement_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;
using game.actors.decoration.movable.turrets.turret;

[
	Name("Turret"), 
	Description("Personal automated self-defence system!") 
]
public class Weapon_Turret : Placement_Weapon
{	
	Weapon_Turret()
	{
		Name = Locale.Get("#item_turret_name");

		Walk_Animation 	= "avatar_body_idle";

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
			Vec2(29, 53),		// S
			Vec2(24, 54),		// SE
			Vec2(37, 50),		// E
			Vec2(40, 46),		// NE
			Vec2(37, 43),		// N
			Vec2(23, 46),		// NW
			Vec2(27, 49),		// W
			Vec2(32, 53)		// SW
		};  
		
		Accepted_Ammo_Types = { 
			typeof(Weapon_Ammo_Explosive),
			typeof(Weapon_Ammo_Freeze),
			typeof(Weapon_Ammo_Incendiary)
		};  

		Slot					= Weapon_Slot.Grenade;
		SubType					= Weapon_SubType.Turret;
		Speed_Multiplier		= 1.0;

		Screenshake_Duration	= 0;
		Screenshake_Intensity	= 0;
		
		Clip_Size				= 1;
		Reserve_Size			= 1;
		Initial_Ammo			= 1;
		Reload_Duration			= 0.500;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.500;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Can_Buffs_Regenerate_Ammo	= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_turret";
		HUD_Ammo_Icon			= "game_hud_ammoicon_turret";
		
		Fire_Sound				= "sfx_objects_turrets_dispose";
		
		AI_Priority				= 0.25f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Turret));

		// Placement specific settings.
		Object_Type				= typeof(Turret);	
        Max_Placed              = 2;

        Pickup_Sprite   		= "actor_pickups_turret_0";
	    Pickup_Name 	    	= Locale.Get("#item_turret_name");
	    Pickup_Sound		    = "sfx_objects_pickups_turret_pickup";		
	}	
}
