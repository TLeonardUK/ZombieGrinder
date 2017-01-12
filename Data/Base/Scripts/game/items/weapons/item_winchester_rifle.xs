// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.rifle.weapon_winchester_rifle;

public class Item_Winchester_Rifle : Item_Archetype
{
	Item_Winchester_Rifle()
	{
		Name 					= "#item_winchester_rifle_name";
		Description 			= "#item_winchester_rifle_description";
		Category				= "#item_winchester_rifle_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
		Unlock_Criteria_Threshold			= 18;
		
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
		
		Icon_Animation			= "item_weapon_winchester_rifle";
		Icon_Offset				= Vec2(16, 10);
		 
		Body_Animation			= "avatar_body_winchester_rifle";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
        Override_Inventory_ID_Name = "Item_Winchester_Rifle_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Winchester_Rifle_Upgrade_Tree));
	
		Weapon_Type				= typeof(Weapon_Winchester_Rifle);
		
		Combine_Name			= "Winchester Rifle";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
