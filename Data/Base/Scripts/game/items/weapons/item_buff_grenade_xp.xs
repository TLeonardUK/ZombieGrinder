// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.grenade.weapon_buff_grenade_xp;

public class Item_Buff_Grenade_Xp : Item_Archetype
{
	Item_Buff_Grenade_Xp()
	{
		Name 					= "#item_buff_grenade_xp_name";
		Description 			= "#item_buff_grenade_xp_description";
		Category				= "#item_buff_grenade_xp_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 25000;
		Unlock_Criteria_Threshold			= 7;
		
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
		
		Icon_Animation			= "item_weapon_buff_grenade_xp";
		Icon_Offset				= Vec2(9, 10);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Body_Animation			= "avatar_body_xp_grenade";
		
		Weapon_Type				= typeof(Weapon_Buff_Grenade_XP);
		
        Override_Inventory_ID_Name = "Item_Buff_Grenade_Xp_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Buff_Grenade_Xp_Upgrade_Tree));
		
		Combine_Name			= "Buff Grenade - XP";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
