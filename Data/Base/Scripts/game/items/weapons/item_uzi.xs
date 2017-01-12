// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.shotgun.weapon_uzi;

public class Item_Uzi : Item_Archetype
{
	Item_Uzi()
	{
		Name 					= "#item_uzi_name";
		Description 			= "#item_uzi_description";
		Category				= "#item_uzi_category"; 
		
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
		
		Icon_Animation			= "item_weapon_uzi";
		Icon_Offset				= Vec2(10, 10);
		
		Body_Animation			= "avatar_body_uzi";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
        
        Override_Inventory_ID_Name = "Item_Uzi_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Uzi_Upgrade_Tree));
	
		Weapon_Type				= typeof(Weapon_Uzi);
		
		Combine_Name			= "Uzi";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[2];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
