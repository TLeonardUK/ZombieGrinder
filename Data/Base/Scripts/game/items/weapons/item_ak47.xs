// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.rifle.weapon_ak47;

public class Item_Ak47 : Item_Archetype
{
	Item_Ak47()
	{
		Name 					= "#item_ak47_name";
		Description 			= "#item_ak47_description";
		Category				= "#item_ak47_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 35000;
		Unlock_Criteria_Threshold			= 5;
		
		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Uzi);
		Unlock_Criteria_Threshold	= 100;
		
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
		
		Icon_Animation			= "item_weapon_ak47";
		Icon_Offset				= Vec2(16, 10);
				
		Body_Animation			= "avatar_body_ak47";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Ak47);
		
        Override_Inventory_ID_Name = "Item_Ak47_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Ak47_Upgrade_Tree));
		
		Combine_Name			= "AK47";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
