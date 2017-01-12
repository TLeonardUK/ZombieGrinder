// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.weapons.placement_weapon;
using game.weapons.ammo.weapon_ammo_explosive;
using game.weapons.ammo.weapon_ammo_freeze;
using game.weapons.ammo.weapon_ammo_incendiary;
using game.actors.decoration.movable.turrets.turret;

[
	Name("Barricade"), 
	Description("Randomised barricade item!") 
]
public class Weapon_Barricade : Placement_Weapon
{	
	Weapon_Barricade()
	{
		Name = Locale.Get("#item_barricade_name");

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
		
		Clip_Size				= 5;
		Reserve_Size			= 10;
		Initial_Ammo			= 10;
		Reload_Duration			= 0.500;
		Has_Unlimited_Ammo	 	= false;
		Fire_Interval			= 0.500;			
		Fire_Ammo_Usage			= 1;
		Is_Rechargable			= false;

		Can_Buffs_Regenerate_Ammo	= false;

		Hear_Radius				= 72.0f;
		
		HUD_Icon				= "game_hud_weaponicon_barricade";
		HUD_Ammo_Icon			= "game_hud_ammoicon_barricade";
		
		Fire_Sound				= "sfx_objects_turrets_dispose";
		
		AI_Priority				= 0.25f;

		Item_Type 				= Item_Archetype.Find_By_Type(typeof(Item_Barricade));

		// Placement specific settings.
		Object_Type				= typeof(Movable);	
        Max_Placed              = 16;	

		Pickup_Sprite	        = "actor_pickups_Barricade_0";
		Pickup_Name		        = Locale.Get("#item_barricade_name");
		Pickup_Sound	        = "sfx_objects_pickups_barricade_pickup";		
	}	

    protected override void On_Movable_Spawned(Movable movable)
    {
        movable.Owner_ID = -1;
        movable.Team_ID = -1;

        switch (Math.Rand(0, 4))
        {
            case 0:
            {
		        movable.Pickup_Sound		= "sfx_objects_destroyable_move_start";
		        movable.Pickup_Effect		= "";
		        movable.Drop_Sound			= "sfx_objects_destroyable_move_end";
		        movable.Drop_Effect			= "dust_puff";
		        movable.Destroy_Sound		= "sfx_objects_destroyable_poof";
		        movable.Destroy_Effect		= "wood_destroy";
		        movable.Bounding_Box		= Vec4(0, 0, 32, 32);
		        movable.Collision_Box 		= Vec4(0, 16, 32, 16);
		        movable.Set_Sprite_Frame("decal_crate_crate_1_0");
                break;
            }
            case 1:
            {
		        movable.Pickup_Sound		= "sfx_objects_destroyable_move_start";
		        movable.Pickup_Effect		= "";
		        movable.Drop_Sound			= "sfx_objects_destroyable_move_end";
		        movable.Drop_Effect			= "dust_puff";
		        movable.Destroy_Sound		= "";
		        movable.Destroy_Effect		= "explosive_destroy";
		        movable.Bounding_Box	    = Vec4(0, 0, 16, 32);
		        movable.Collision_Box 	    = Vec4(0, 16, 16, 16);
		        movable.Set_Sprite_Frame("decal_barrel_all_colors_9");
                break;
            }
            case 2:
            {
		        movable.Pickup_Sound		= "sfx_objects_destroyable_move_start";
		        movable.Pickup_Effect		= "";
		        movable.Drop_Sound			= "sfx_objects_destroyable_move_end";
		        movable.Drop_Effect			= "dust_puff";
		        movable.Destroy_Sound		= "";
		        movable.Destroy_Effect		= "incendiary_destroy";
		        movable.Bounding_Box		= Vec4(0, 0, 16, 32);
		        movable.Collision_Box 		= Vec4(0, 16, 16, 16);
		        movable.Set_Sprite_Frame("decal_barrel_all_colors_10");
                break;
            }
            case 3:
            {
		        movable.Pickup_Sound		= "sfx_objects_destroyable_move_start";
		        movable.Pickup_Effect		= "";
		        movable.Drop_Sound			= "sfx_objects_destroyable_move_end";
		        movable.Drop_Effect			= "dust_puff";
		        movable.Destroy_Sound		= "sfx_objects_destroyable_poof";
		        movable.Destroy_Effect		= "metal_destroy";
		        movable.Bounding_Box		= Vec4(0, 0, 16, 32);
		        movable.Collision_Box 		= Vec4(0, 16, 16, 16);
		        movable.Set_Sprite_Frame("decal_barrel_all_colors_0");
                break;
            }
        }
    }
}
