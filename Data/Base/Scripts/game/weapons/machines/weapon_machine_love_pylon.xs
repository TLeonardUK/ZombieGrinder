// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.projectile_weapon;

[
	Name("Love Pylon"), 
	Description("Such bullets, much wow.") 
]
public class Weapon_Machine_Love_Pylon : Projectile_Weapon
{		
	Weapon_Machine_Love_Pylon()
	{
		Name = Locale.Get("#item_love_pylon_name");

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
			Vec2(23, 9),		// S
			Vec2(23, 9),		// SE
			Vec2(23, 9),		// E
			Vec2(23, 9),		// NE
			Vec2(23, 9),		// N
			Vec2(23, 9),		// NW
			Vec2(23, 9),		// W
			Vec2(23, 9)	    	// SW
		};  
		
		SubType					= Weapon_SubType.Healing;

		Clip_Size				= 1000;
		Reserve_Size			= 0;
		Initial_Ammo			= 0;
		Reload_Duration			= 0.0;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.035f;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;
		Use_Individual_Reload	= false;
		
		Fire_Sound				= "";
		Dry_Fire_Sound			= "";
		
		AI_Priority				= 0.75f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Love_Pylon));

		// Projectile specific settings.
		Muzzle_Effect			= "love_pylon_beam";
		Is_Automatic			= true;
        
	    Pickup_Sprite		    = "actor_pickups_love_pylon_0";
	    Pickup_Name		        = Locale.Get("#item_love_pylon_name");
	    Pickup_Sound		    = "sfx_objects_pickups_turret_pickup";
	}
}
