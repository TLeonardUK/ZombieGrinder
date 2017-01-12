// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Turret"), 
	Description("Such bullets, much wow.") 
]
public class Weapon_Machine_Explosive_Turret : Projectile_Weapon
{		
	Weapon_Machine_Explosive_Turret()
	{
		Name = Locale.Get("#item_turret_name");

		Walk_Animation 	= "";
		Fire_Animation	= "";

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
			Vec2(24, 42),		// S
			Vec2(38, 36),		// SE
			Vec2(42, 23),		// E
			Vec2(38, 10),		// NE
			Vec2(24, 4),		// N
			Vec2(11, 10),		// NW
			Vec2(6, 23),		// W
			Vec2(11, 35)		// SW
		};  
		
		SubType					= Weapon_SubType.Turret;

		Clip_Size				= 50;
		Reserve_Size			= 0;
		Initial_Ammo			= 0;
		Reload_Duration			= 0.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 1.5;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		
		Fire_Sound				= "sfx_objects_weapons_rocket_launcher_fire";
		Reload_Sound			= "sfx_objects_weapons_rocket_launcher_reload";
		
		AI_Priority				= 0.75f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Turret));

		// Projectile specific settings.
		Muzzle_Effect			= "turret_explosive_fire";
		Is_Automatic			= true;
        
	    Pickup_Sprite		    = "actor_pickups_turret_0";
	    Pickup_Name		        = Locale.Get("#item_turret_name");
	    Pickup_Sound		    = "sfx_objects_pickups_turret_pickup";
	}
}
