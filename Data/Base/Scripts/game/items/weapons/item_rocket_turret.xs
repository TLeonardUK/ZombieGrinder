// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Rocket_Turret : Item_Archetype
{
	Item_Rocket_Turret()
	{
		Name 					= "#item_rocket_turret_name";
		Description 			= "#item_rocket_turret_description";
		Category				= "#item_rocket_turret_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 60000;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV100";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_rocket_turret";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_idle";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Rocket_Turret);
		
		Combine_Name			= "Rocket Turret";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
