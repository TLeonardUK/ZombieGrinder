// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.melee.weapon_fight_saber;

public class Item_Fight_Saber : Item_Archetype
{
	Item_Fight_Saber()
	{
		Name 					= "#item_fight_saber_name";
		Description 			= "#item_fight_saber_description";
		Category				= "#item_fight_saber_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 50000;
		
		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Baseball_Bat);
		Unlock_Criteria_Threshold	= 100;
		
		Is_Stackable			= false;
		Is_Buyable				= true;
		Is_Tintable				= true;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= true;
		Is_PVP_Usable			= true;
		Is_Inventory_Droppable	= false;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV100";
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_fight_saber";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_fight_saber";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Fight_Saber);
		
        Override_Inventory_ID_Name = "Item_Fight_Saber_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Fight_Saber_Upgrade_Tree));
		
		Combine_Name			= "Fight Saber";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Paints", Item_Combine_Method.Tint, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
