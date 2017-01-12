// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_flamethrower;

public class Item_Flamethrower : Item_Archetype
{
	Item_Flamethrower()
	{
		Name 					= "#item_flamethrower_name";
		Description 			= "#item_flamethrower_description";
		Category				= "#item_flamethrower_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
		
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

		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Molotov);
		Unlock_Criteria_Threshold	= 100;
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_flamethrower";
		Icon_Offset				= Vec2(12, 10);
		
		Body_Animation			= "avatar_body_flamethrower";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Flamethrower);
		
        Override_Inventory_ID_Name = "Item_Flamethrower_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Flamethrower_Upgrade_Tree));
		
		Combine_Name			= "Flamethrower";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
