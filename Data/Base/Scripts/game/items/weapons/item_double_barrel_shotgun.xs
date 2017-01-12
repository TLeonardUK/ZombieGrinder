// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.shotgun.weapon_db_shotgun;

public class Item_Double_Barrel_Shotgun : Item_Archetype
{
	Item_Double_Barrel_Shotgun()
	{
		Name 						= "#item_double_barrel_shotgun_name";
		Description 				= "#item_double_barrel_shotgun_description";
		Category					= "#item_double_barrel_shotgun_category";
		
		Slot						= Item_Slot.Weapon;
		Cost						= 30000;
		
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
		
		Rarity					= Item_Rarity.Common;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_weapon_double_barrel_shotgun";
		Icon_Offset				= Vec2(16, 16);
		
		Body_Animation			= "avatar_body_dbshotgun";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_DB_Shotgun);
		
        Override_Inventory_ID_Name = "Item_Double_Barrel_Shotgun_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Double_Barrel_Shotgun_Upgrade_Tree));
		
		Combine_Name			= "Double Barrel Shotgun";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
