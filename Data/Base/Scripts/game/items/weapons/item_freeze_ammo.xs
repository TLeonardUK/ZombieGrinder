// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.ammo.weapon_ammo_freeze;

public class Item_Freeze_Ammo : Item_Archetype
{
	Item_Freeze_Ammo()
	{
		Name 					= "#item_freeze_ammo_name";
		Description 			= "#item_freeze_ammo_description";
		Category				= "#item_freeze_ammo_category";
		
		Slot					= Item_Slot.All;
		Cost					= 30000;
		Unlock_Criteria_Threshold			= 5;
		
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
		
		Icon_Animation			= "item_weapon_freezeammo";
		Icon_Offset				= Vec2(16, 20);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Ammo_Type				= typeof(Weapon_Ammo_Freeze);
		
		Combine_Name			= "Freeze Ammo";
		Combine_Group			= "Ammo";
		Combinations 			= new Item_Combination[0];
	}
}
