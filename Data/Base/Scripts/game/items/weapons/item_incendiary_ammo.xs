// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.ammo.weapon_ammo_incendiary;

public class Item_Incendiary_Ammo : Item_Archetype
{
	Item_Incendiary_Ammo()
	{
		Name 					= "#item_incendiary_ammo_name";
		Description 			= "#item_incendiary_ammo_description";
		Category				= "#item_incendiary_ammo_category";
		
		Slot					= Item_Slot.All;
		Cost					= 30000;
		Unlock_Criteria_Threshold			= 13;
		
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
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_incendiaryammo";
		Icon_Offset				= Vec2(16, 20);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Ammo_Type				= typeof(Weapon_Ammo_Incendiary);
		
		Combine_Name			= "Incendiary Ammo";
		Combine_Group			= "Ammo";
		Combinations 			= new Item_Combination[0];
	}
}
