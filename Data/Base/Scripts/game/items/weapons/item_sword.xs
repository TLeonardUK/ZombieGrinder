// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.melee.weapon_sword;

public class Item_Sword : Item_Archetype
{
	Item_Sword()
	{
		Name 					= "#item_sword_name";
		Description 			= "#item_sword_description";
		Category				= "#item_sword_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 50000;
		
		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Baseball_Bat);
		Unlock_Criteria_Threshold	= 1000;
		
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
		
		Icon_Animation			= "item_weapon_sword";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_sword";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
        Override_Inventory_ID_Name = "Item_Sword_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Sword_Upgrade_Tree));
	
		Weapon_Type				= typeof(Weapon_Sword);
		
		Combine_Name			= "Sword";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
