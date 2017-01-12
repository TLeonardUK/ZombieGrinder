// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.placement_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;
using game.actors.decoration.movable.turrets.turret;

[
	Name("Love Pylon"), 
	Description("Personal automated self-defence system!") 
]
public class Weapon_Love_Pylon : Placement_Weapon
{	
	Weapon_Love_Pylon()
	{
		Name = Locale.Get("#item_love_pylon_name");

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
		
		Slot					= Weapon_Slot.Grenade;
		SubType					= Weapon_SubType.Heavy;
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
		
		HUD_Icon				= "game_hud_weaponicon_love_pylon";
		HUD_Ammo_Icon			= "game_hud_ammoicon_love";
		
		Fire_Sound				= "sfx_objects_turrets_dispose";
		
		AI_Priority				= 0.25f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Love_Pylon));

		// Placement specific settings.
		Object_Type				= typeof(Love_Pylon);	
        Max_Placed              = 2;	
        
	    Pickup_Sprite	    	= "actor_pickups_love_pylon_0";
	    Pickup_Name 	    	= Locale.Get("#item_love_pylon_name");
	    Pickup_Sound	    	= "sfx_objects_pickups_turret_pickup";		
	}	
}
