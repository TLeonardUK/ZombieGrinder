// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.pistol.weapon_pistol;

public class Item_Pistol : Item_Archetype
{
	Item_Pistol()
	{
		Name 					= "#item_pistol_name";
		Description 			= "#item_pistol_description";
		Category				= "#item_pistol_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 5000;
		
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
		
		Icon_Animation			= "item_weapon_pistol";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_pistol";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Pistol);
		
        Override_Inventory_ID_Name = "Item_Pistol_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Pistol_Upgrade_Tree));
		
		Combine_Name			= "Pistol";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[3];
		Combinations[0] 		= Item_Combination("Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[1] 		= Item_Combination("Confetti Ammo", Item_Combine_Method.Attach, "Nothing");
		Combinations[2] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
