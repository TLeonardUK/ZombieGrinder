// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_rpc;

public class Item_Rpc : Item_Archetype
{
	Item_Rpc()
	{
		Name 					= "#item_rpc_name";
		Description 			= "#item_rpc_description";
		Category				= "#item_rpc_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 50000;
		Unlock_Criteria_Threshold			= 16;
		
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
		
		Icon_Animation			= "item_weapon_rpc";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_rpc";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_RPC);
		
        Override_Inventory_ID_Name = "Item_Rpc_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Rpc_Upgrade_Tree));
		
		Combine_Name			= "Rocket Propelled Chainsaw";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
