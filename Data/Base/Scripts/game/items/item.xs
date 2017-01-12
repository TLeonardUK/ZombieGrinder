// ---------------------------------------------N--------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log;
using game.items.item_archetype;

[
	Name("Item"), 
	Description("An item is an individual inventory item. It essentially stores a reference to an item-archetype as well as customization properties (color tint, etc).") 
]
public native("Item") class Item
{
	public native("Create") Item();
	
	// Item archetype, describes generic item information.
	property Item_Archetype Archetype
	{
		public native("Get_Archetype") Item_Archetype Get();
		public native("Set_Archetype") void Set(Item_Archetype value);	
	}
	
	// Item ID, used purely for serialization.
	property serialized(1) int Archetype_ID
	{
		public native("Get_Archetype_ID") int Get();
		public native("Set_Archetype_ID") void Set(int value);	
	}
 
	// Unique ID generated when item is recieved.
	property serialized(1) int Unique_ID
	{
		public native("Get_Unique_ID") int Get();
		public native("Set_Unique_ID") void Set(int value);		
	}
	 
	// Gets the ID of the item we are attached to.
	property serialized(1) int Attached_To_ID
	{
		public native("Get_Attached_To_ID") int Get();
		public native("Set_Attached_To_ID") void Set(int value);		
	}
	
	// Time when item was recieved/purchased/etc.
	property serialized(1) int Recieve_Time
	{
		public native("Get_Recieve_Time") int Get();
		public native("Set_Recieve_Time") void Set(int value);		
	}
	
	// Primary tint color of item.
	property serialized(1) Vec4	Primary_Color
	{
		public native("Get_Primary_Color") Vec4 Get();
		public native("Set_Primary_Color") void Set(Vec4 value);		
	}
	
	// Time when item was equipped.
	property serialized(1) int Equip_Time
	{
		public native("Get_Equip_Time") int Get();
		public native("Set_Equip_Time") void Set(int value);		
	}
	
	// Slot where this item is equipped to.
	property serialized(1) Item_Slot Equip_Slot
	{
		public native("Get_Equip_Slot") Item_Slot Get();
		public native("Set_Equip_Slot") void Set(Item_Slot value);		
	}

	// True if was dropped by inventory system.
	property serialized(700) bool Was_Inventory_Drop
	{
		public native("Get_Was_Inventory_Drop") bool Get();
		public native("Set_Was_Inventory_Drop") void Set(bool value);		
	}

	// ID to link to inventory drop.
	property serialized(700) string Inventory_Drop_ID
	{
		public native("Get_Inventory_Drop_ID") string Get();
		public native("Set_Inventory_Drop_ID") void Set(string value);		
	}

	// ID to link to inventory drop.
	property serialized(701) string Inventory_Original_Drop_ID
	{
		public native("Get_Inventory_Original_Drop_ID") string Get();
		public native("Set_Inventory_Original_Drop_ID") void Set(string value);		
	}	

    // How long in seconds this item has been equipped on the player (only increments during game-play).
    property serialized(999) float Equip_Duration
    {
		public native("Get_Equip_Duration") float Get();
		public native("Set_Equip_Duration") void Set(float value);		
    }

    // If true the item is indestuctable. This is true for items found via challenges/chests/steam-drops
    property serialized(999) bool Is_Indestructable
    {
		public native("Get_Is_Indestructable") bool Get();
		public native("Set_Is_Indestructable") void Set(bool value);		
    }

    // An array of numeric ids that correspond to upgrades in the weapons upgrade tree.
    property serialized(4000) int[] Upgrade_Ids
    {
		public native("Get_Upgrade_Ids") int[]  Get();
		public native("Set_Upgrade_Ids") void Set(int[]  value);		
    }
}