// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Large_Gem_Grab_Bag : Item_Archetype
{
	Item_Large_Gem_Grab_Bag()
	{
		Name 					= "#item_large_gem_grab_bag_name";
		Description 			= "#item_large_gem_grab_bag_description";
		Category				= "#item_large_gem_grab_bag_category";
		
		Slot					= Item_Slot.All;
		Cost					= 10000; 
		
		Is_Stackable			= true;
		Is_Buyable				= false;
		Is_Tintable				= false;
		Is_Sellable				= true;
		Is_Tradable				= true;
		Is_Dropable				= false;
		Is_PVP_Usable			= false;
		Is_Inventory_Droppable	= true;
		Is_Unpackable			= true;
		Is_Premium_Only			= false;
		Premium_Price			= "1;VLV200";
		
		Rarity					= Item_Rarity.Legendary;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_large_gem_grab_bag";
		Icon_Offset				= Vec2(8, 11);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Large Coin Gem Bag";
		Combine_Group			= "Premium";
		Combinations 			= new Item_Combination[0];
	}
			
	public override void On_Unpack(Profile profile, Item item)
	{ 
		Large_Gem_Grab_Bag_Drop_Table table = new Large_Gem_Grab_Bag_Drop_Table();
		for (int i = 0; i < 3; i++)
		{
			profile.Add_Unpacked_Item(Item_Archetype.Find_By_Type(table.Get_Drop()));
		}
	}
}
