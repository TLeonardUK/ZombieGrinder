// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

public class Item_Medium_Coin_Grab_Bag : Item_Archetype
{
	Item_Medium_Coin_Grab_Bag()
	{
		Name 					= "#item_medium_coin_grab_bag_name";
		Description 			= "#item_medium_coin_grab_bag_description";
		Category				= "#item_medium_coin_grab_bag_category";
		
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
		Premium_Price			= "1;VLV150";
		
		Rarity					= Item_Rarity.Ultra_Rare;
		
		Default_Tint			= Vec4(255, 255, 255, 255);
		
		Icon_Animation			= "item_medium_coin_grab_bag";
		Icon_Offset				= Vec2(8, 11);
		
		Is_Icon_Direction_Based	= false;
		Is_Icon_Overlay			= false;
		
		Combine_Name			= "Medium Coin Grab Bag";
		Combine_Group			= "Premium";
		Combinations 			= new Item_Combination[0];
	}
	
	public override void On_Unpack(Profile profile, Item item)
	{
		profile.Add_Unpacked_Coins(Math.Rand(50000, 100000));
	}
}
