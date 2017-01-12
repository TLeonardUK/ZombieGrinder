// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.shotgun.weapon_shotgun;

public class Item_Shotgun : Item_Archetype
{
	Item_Shotgun()
	{
		Name 					= "#item_shotgun_name";
		Description 			= "#item_shotgun_description";
		Category				= "#item_shotgun_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 20000;
		
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
		
		Icon_Animation			= "item_weapon_shotgun";
		Icon_Offset				= Vec2(10, 10);
		
		Body_Animation			= "avatar_body_shotgun";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Shotgun);
		
        Override_Inventory_ID_Name = "Item_Shotgun_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Shotgun_Upgrade_Tree));
		
		Combine_Name			= "Shotgun";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
