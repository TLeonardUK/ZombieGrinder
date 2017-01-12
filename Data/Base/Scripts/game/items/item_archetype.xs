// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log; 
using game.profile.profile;
using game.items.item;
   
public enum Item_Combine_Method
{
	Tint		= 0,		// Changes item color
	Attach		= 1,		// Attachs item as a "sub-item" of the destination item.
	Merge		= 2,		// Merges the item and produces a new item of Result_Item type.,
	Repair		= 3,		// Repairs durability.
	
	COUNT
}

public enum Item_Slot
{
	Not_Equipped = -1,
	Head		= 0,
	Accessory	= 1,
	Weapon		= 2,
	All			= 3,
	
	COUNT
}

public enum Item_Rarity
{
	Common		= 0,
	Uncommon	= 1,
	Rare		= 2,
	Ultra_Rare	= 3,
	Legendary	= 4,
	
	COUNT
}

public enum Item_Unlock_Criteria
{
	Required_Rank = 0,
	Kills_With_Weapon = 1,
	Damage_With_Weapon = 2,
	Ailments_From_Weapon = 3
}

public native("Item_Combination") struct Item_Combination
{
	property string Other_Group
	{
		public native("Get_Other_Group") string Get();
		public native("Set_Other_Group") void Set(string value);	
	}
	
	property Item_Combine_Method Combine_Method
	{
		public native("Get_Combine_Method") Item_Combine_Method Get();
		public native("Set_Combine_Method") void Set(Item_Combine_Method value);	
	}
	
	property string Result_Item
	{
		public native("Get_Result_Item") string Get();
		public native("Set_Result_Item") void Set(string value);	
	}

	public native("Create") Item_Combination(string other, Item_Combine_Method method, string result);
}

[
	Name("Item_Archetype"), 
	Description("An item archetype defines all the shared properties of an item.") 
]
public native("Item_Archetype") class Item_Archetype 
{
	property string Name
	{
		public native("Get_Name") string Get();
		public native("Set_Name") void Set(string value);	
	}
	
    property string Varient_Prefix
	{
		public native("Get_Varient_Prefix") string Get();
		public native("Set_Varient_Prefix") void Set(string value);	
	}    

	property string Description
	{
		public native("Get_Description") string Get();
		public native("Set_Description") void Set(string value);	
	}
	
	property string Category
	{
		public native("Get_Category") string Get();
		public native("Set_Category") void Set(string value);	
	}
	
	property Item_Slot Slot
	{
		public native("Get_Slot") Item_Slot Get();
		public native("Set_Slot") void Set(Item_Slot value);	
	}
	
	property int Cost
	{
		public native("Get_Cost") int Get();
		public native("Set_Cost") void Set(int value);	
	}
	
	property bool Is_Stackable
	{
		public native("Get_Is_Stackable") bool Get();
		public native("Set_Is_Stackable") void Set(bool value);	
	}
	
	property bool Is_Buyable
	{
		public native("Get_Is_Buyable") bool Get();
		public native("Set_Is_Buyable") void Set(bool value);	
	}
	
	property bool Is_Tintable
	{
		public native("Get_Is_Tintable") bool Get();
		public native("Set_Is_Tintable") void Set(bool value);	
	}
	
	property bool Is_Sellable
	{
		public native("Get_Is_Sellable") bool Get();
		public native("Set_Is_Sellable") void Set(bool value);	
	}
	
	property bool Is_Tradable
	{
		public native("Get_Is_Tradable") bool Get();
		public native("Set_Is_Tradable") void Set(bool value);	
	}
	
	property bool Is_Dropable
	{
		public native("Get_Is_Dropable") bool Get();
		public native("Set_Is_Dropable") void Set(bool value);	
	}
	
	property bool Is_Inventory_Droppable
	{
		public native("Get_Is_Inventory_Droppable") bool Get();
		public native("Set_Is_Inventory_Droppable") void Set(bool value);	
	}
	
	property bool Is_PVP_Usable
	{
		public native("Get_Is_PVP_Usable") bool Get();
		public native("Set_Is_PVP_Usable") void Set(bool value);	
	}
	
	property bool Is_Unpackable
	{
		public native("Get_Is_Unpackable") bool Get();
		public native("Set_Is_Unpackable") void Set(bool value);	
	}
	
	property int Max_Stack
	{
		public native("Get_Max_Stack") int Get();
		public native("Set_Max_Stack") void Set(int value);	
	}
	
	property Item_Rarity Rarity
	{
		public native("Get_Rarity") Item_Rarity Get();
		public native("Set_Rarity") void Set(Item_Rarity value);	
	}
	
	property Vec4 Default_Tint
	{
		public native("Get_Default_Tint") Vec4 Get();
		public native("Set_Default_Tint") void Set(Vec4 value);	
	}
	
	property string Body_Animation
	{
		public native("Get_Body_Animation") string Get();
		public native("Set_Body_Animation") void Set(string value);	
	}
	
	property string Icon_Animation
	{
		public native("Get_Icon_Animation") string Get();
		public native("Set_Icon_Animation") void Set(string value);	
	}
	
	property Vec2 Icon_Offset
	{
		public native("Get_Icon_Offset") Vec2 Get();
		public native("Set_Icon_Offset") void Set(Vec2 value);	
	}
	
	property bool Is_Icon_Direction_Based
	{
		public native("Get_Is_Icon_Direction_Based") bool Get();
		public native("Set_Is_Icon_Direction_Based") void Set(bool value);	
	}
	
	property bool Is_Icon_Overlay
	{
		public native("Get_Is_Icon_Overlay") bool Get();
		public native("Set_Is_Icon_Overlay") void Set(bool value);	
	} 

	property bool Is_Premium_Only
	{
		public native("Get_Is_Premium_Only") bool Get();
		public native("Set_Is_Premium_Only") void Set(bool value);	
	} 
	
	property string Premium_Price
	{
		public native("Get_Premium_Price") string Get();
		public native("Set_Premium_Price") void Set(string value);	
	} 
	
	property string Combine_Name
	{
		public native("Get_Combine_Name") string Get();
		public native("Set_Combine_Name") void Set(string value);	
	}
	
	property string Combine_Group
	{
		public native("Get_Combine_Group") string Get();
		public native("Set_Combine_Group") void Set(string value);	
	}

	property bool Use_Custom_Color
	{
		public native("Get_Use_Custom_Color") bool Get();
		public native("Set_Use_Custom_Color") void Set(bool value);	
	}

	property float Initial_Armour_Amount
	{
		public native("Get_Initial_Armour_Amount") float Get();
		public native("Set_Initial_Armour_Amount") void Set(float value);	
	}
    
	property Item_Upgrade_Tree Upgrade_Tree
	{
		public native("Get_Upgrade_Tree") Item_Upgrade_Tree Get();
		public native("Set_Upgrade_Tree") void Set(Item_Upgrade_Tree value);	
	}    

	// Unlock criterial.
	property Item_Unlock_Criteria Unlock_Criteria
	{
		public native("Get_Unlock_Criteria") Item_Unlock_Criteria Get();
		public native("Set_Unlock_Criteria") void Set(Item_Unlock_Criteria value);	
	}
	
	property Type Unlock_Criteria_Item
	{
		public native("Get_Unlock_Criteria_Item") Type Get();
		public native("Set_Unlock_Criteria_Item") void Set(Type value);	
	}
	
	property int Unlock_Criteria_Threshold
	{
		public native("Get_Unlock_Criteria_Threshold") int Get();
		public native("Set_Unlock_Criteria_Threshold") void Set(int value);	
	}

    // This is in seconds for how long this item can be in-use before it breaks.
    // Inventory items or indestructable items ignore this.
	property float Max_Durability
	{
		public native("Get_Max_Durability") float Get();
		public native("Set_Max_Durability") void Set(float value);	
	}
	
	property Type Weapon_Type
	{
		public native("Get_Weapon_Type") Type Get();
		public native("Set_Weapon_Type") void Set(Type type);
	}
	
	property Type Ammo_Type
	{
		public native("Get_Ammo_Type") Type Get();
		public native("Set_Ammo_Type") void Set(Type type);
	}

	property Type Post_Process_FX_Type
	{
		public native("Get_Post_Process_FX_Type") Type Get();
		public native("Set_Post_Process_FX_Type") void Set(Type type);
	}
	
	property Item_Combination[] Combinations
	{
		public native("Get_Combinations") Item_Combination[] Get();
		public native("Set_Combinations") void Set(Item_Combination[] value);	
	}
	
    property string Override_Inventory_ID_Name
    {
		public native("Get_Override_Inventory_ID_Name") string Get();
		public native("Set_Override_Inventory_ID_Name") void Set(string value);	
    }

	public virtual void On_Recieve(Profile profile, Item item)
	{	
		// Do stuff here if we have special-requirements. Giving user item etc.		
		Log.Write("Item.On_Recieve = " + item.Archetype.Name);
	}
	
	public virtual void On_Unpack(Profile profile, Item item)
	{	
		// Do stuff here if we have special-requirements. Giving user item etc.		
		Log.Write("Item.On_Unpack = " + item.Archetype.Name);
	}
	
	public static native("Find_By_Name") Item_Archetype Find_By_Name(string name);
	public static native("Find_By_Type") Item_Archetype Find_By_Type(Type id);
}
