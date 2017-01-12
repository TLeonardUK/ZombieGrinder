// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.placement_weapon;
using game.actors.decoration.movable.mines.tripmine;

[
	Name("Tripmine"), 
	Description("Player+Laser=Kaboom.") 
]
public class Weapon_Tripmine : Placement_Weapon
{	
	Weapon_Tripmine()
	{
		Name = Locale.Get("#item_tripmine_name");

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
		Reserve_Size			= 10;
		Initial_Ammo			= 10;
		Reload_Duration			= 0.500;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.500;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Can_Buffs_Regenerate_Ammo	= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_tripmine";
		HUD_Ammo_Icon			= "game_hud_ammoicon_tripmine";
		
		Fire_Sound				= "sfx_objects_turrets_dispose";
		
		AI_Priority				= 0.25f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Tripmine));

		// Placement specific settings.
		Object_Type				= typeof(Tripmine);	
        Max_Placed              = 8;	

		Pickup_Sprite	= "actor_pickups_tripmine_0";
		Pickup_Name		= Locale.Get("#item_tripmine_name");
		Pickup_Sound	= "sfx_objects_pickups_tripmine_pickup";
	}	
}
