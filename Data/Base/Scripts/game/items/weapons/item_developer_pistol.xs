// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.pistol.weapon_developer_pistol;

public class Item_Developer_Pistol : Item_Archetype
{
	Item_Developer_Pistol()
	{
		Name 					= "#item_developer_pistol_name";
		Description 			= "#item_developer_pistol_description";
		Category				= "#item_developer_pistol_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 0;
		
		Is_Stackable			= false;
		Is_Buyable				= false;
		Is_Tintable				= false;
		Is_Sellable				= false;
		Is_Tradable				= false;
		Is_Dropable				= false;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_magnum";
		Icon_Offset				= Vec2(12, 12);
		
		Body_Animation			= "avatar_body_pistol";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Developer_Pistol);
		
		Combine_Name			= "Developer Pistol";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
