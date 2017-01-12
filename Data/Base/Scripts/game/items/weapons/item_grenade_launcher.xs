// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;
using game.weapons.heavy.weapon_grenade_launcher;

public class Item_Grenade_Launcher : Item_Archetype
{
	Item_Grenade_Launcher()
	{
		Name 					= "#item_grenade_launcher_name";
		Description 			= "#item_grenade_launcher_description";
		Category				= "#item_grenade_launcher_category";
		
		Slot					= Item_Slot.Weapon;
		Cost					= 40000;
	
		Unlock_Criteria				= Item_Unlock_Criteria.Kills_With_Weapon;
		Unlock_Criteria_Item		= typeof(Item_Frag);
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
		
		Icon_Animation			= "item_weapon_grenade_launcher";
		Icon_Offset				= Vec2(16, 10);
		
		Body_Animation			= "avatar_body_grenadelauncher";
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Weapon_Type				= typeof(Weapon_Grenade_Launcher);
		
        Override_Inventory_ID_Name = "Item_Grenade_Launcher_Old";
        Upgrade_Tree            = Item_Upgrade_Tree.Find_By_Type(typeof(Item_Grenade_Launcher_Upgrade_Tree));
		
		Combine_Name			= "Grenade Launcher";
		Combine_Group			= "Guns";
		Combinations 			= new Item_Combination[1];
		Combinations[0] 		= Item_Combination("Spanner", Item_Combine_Method.Repair, "Nothing");
	}
}
