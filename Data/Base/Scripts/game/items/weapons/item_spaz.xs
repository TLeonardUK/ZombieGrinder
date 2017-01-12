// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.shotgun.weapon_spaz;

public class Item_Spaz : Item_Archetype
{
	Item_Spaz()
	{
		Name 					= "#item_spaz_name";
		Description 			= "#item_spaz_description";
		Category				= "#item_spaz_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
		
		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Shotgun);
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
		
		Rarity					= Item_Rarity.Uncommon;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_spaz";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_spaz";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Spaz);
		
        Override_Inventory_ID_Name = "Item_Spaz_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Spaz_Upgrade_Tree));
		
		Combine_Name			= "Spaz";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
