// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.pistol.weapon_magnum;

public class Item_Magnum : Item_Archetype
{
	Item_Magnum()
	{
		Name 					= "#item_magnum_name";
		Description 			= "#item_magnum_description";
		Category				= "#item_magnum_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 30000;
		Unlock_Criteria_Threshold			= 2;
			
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
		
		Icon_Animation			= "item_weapon_magnum";
		Icon_Offset				= Vec2(12, 12);
		
		Body_Animation			= "avatar_body_magnum";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Magnum);
		
        Override_Inventory_ID_Name = "Item_Magnum_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Magnum_Upgrade_Tree));
		
		Combine_Name			= "Magnum";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
