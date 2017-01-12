// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_love_thrower;

public class Item_Love_Thrower : Item_Archetype
{
	Item_Love_Thrower()
	{
		Name 					= "#item_love_thrower_name";
		Description 			= "#item_love_thrower_description";
		Category				= "#item_love_thrower_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 60000;
	
		Unlock_Criteria				= Item_Unlock_Criteria.Damage_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Love_Cannon);
		Unlock_Criteria_Threshold	= -1000;

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
		
		Icon_Animation			= "item_weapon_love_thrower";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_love_thrower";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Love_Thrower);
		
        Override_Inventory_ID_Name = "Item_Love_Thrower_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Love_Thrower_Upgrade_Tree));
		
		Combine_Name			= "Love Thrower";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
