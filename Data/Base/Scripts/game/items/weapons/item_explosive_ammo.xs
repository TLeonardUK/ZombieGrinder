// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.ammo.weapon_ammo_explosive;

public class Item_Explosive_Ammo : Item_Archetype
{
	Item_Explosive_Ammo()
	{
		Name 					= "#item_explosive_ammo_name";
		Description 			= "#item_explosive_ammo_description";
		Category				= "#item_explosive_ammo_category";
		
		Slot					= Item_Slot.All;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 21;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV100";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_explosiveammo";
		Icon_Offset				= Vec2(16, 20);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Ammo_Type				= typeof(Weapon_Ammo_Explosive);
		
		Combine_Name			= "Explosive Ammo";
		Combine_Group			= "Ammo";
		Combinations 			= new Item_Combination[0];
	}
}
