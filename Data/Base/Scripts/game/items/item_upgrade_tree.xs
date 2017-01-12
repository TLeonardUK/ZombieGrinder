// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using runtime.math;
using runtime.log; 
using game.profile.profile;
using game.items.item;

public enum Item_Upgrade_Tree_Node_Modifier_Type
{
    // Keep in sync with the enum in ItemManager.h
    // Also make sure to update ItemUpgradeTreeNodeModifier::To_Description_String
    // Keep in sync with the enum ParticleUpgradeModifiers in ParticleManager.h as well for simplicity.

    // Base states.
    Damage,
    AmmoCapacity,
    RateOfFire,
    ReloadSpeed,
    ProjectileSpeed,
    Accuracy,
    Penetrating,
    Durability,

    // Specials
    TriBarrel,
    QuadBarrel,
    MultiTap,
    Suicidal
}

// Not happy with this arrangement at all. This is going to bloat the root set for the VM's GC. Blarghal.

public native("Item_Upgrade_Tree_Node_Modifier") class Item_Upgrade_Tree_Node_Modifier
{
    property Item_Upgrade_Tree_Node_Modifier_Type Modifier
	{
		public native("Get_Modifier") Item_Upgrade_Tree_Node_Modifier_Type Get();
		public native("Set_Modifier") void Set(Item_Upgrade_Tree_Node_Modifier_Type value);	
	}
    property float Scale
	{
		public native("Get_Scale") float Get();
		public native("Set_Scale") void Set(float value);	
	}

	public native("Create") Item_Upgrade_Tree_Node_Modifier();	
}

[
	Name("Item_Upgrade_Tree_Node"), 
	Description("Defines an individual node in an upgrade tree.") 
]
public native("Item_Upgrade_Tree_Node") class Item_Upgrade_Tree_Node
{
    property Vec2 Position
	{
		public native("Get_Position") Vec2 Get();
		public native("Set_Position") void Set(Vec2 value);	
	}
    property int ID
	{
		public native("Get_ID") int Get();
	}
    property string Name
	{
		public native("Get_Name") string Get();
		public native("Set_Name") void Set(string value);	
	}
    property Type Gem_Type
	{
		public native("Get_Gem_Type") Type Get();
		public native("Set_Gem_Type") void Set(Type value);	
	}
    property bool Connect_Up
	{
		public native("Get_Connect_Up") bool Get();
		public native("Set_Connect_Up") void Set(bool value);	
	}
    property bool Connect_Down
	{
		public native("Get_Connect_Down") bool Get();
		public native("Set_Connect_Down") void Set(bool value);	
	}
    property bool Connect_Left
	{
		public native("Get_Connect_Left") bool Get();
		public native("Set_Connect_Left") void Set(bool value);	
	}
    property bool Connect_Right
	{
		public native("Get_Connect_Right") bool Get();
		public native("Set_Connect_Right") void Set(bool value);	
	}
    property int Cost
	{
		public native("Get_Cost") int Get();
		public native("Set_Cost") void Set(int value);	
	}
    property Item_Upgrade_Tree_Node_Modifier[] Modifiers
	{
		public native("Get_Modifiers") Item_Upgrade_Tree_Node_Modifier[] Get();
		public native("Set_Modifiers") void Set(Item_Upgrade_Tree_Node_Modifier[] value);	
	}

	public native("Create") Item_Upgrade_Tree_Node();	

    public void Init()
    {
        Modifiers = new Item_Upgrade_Tree_Node_Modifier[0];
    }
    
    public void Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type inType, float Scale)
    {
        Item_Upgrade_Tree_Node_Modifier mod = new Item_Upgrade_Tree_Node_Modifier();
        mod.Modifier = inType;
        mod.Scale = Scale;
        this.Modifiers.AddLast(mod);
    }

    public void Add_Damage_Modifier(float Scale)
    {
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.Damage, Scale);
    }

    public void Add_AmmoCapacity_Modifier(float Scale)
    {
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.AmmoCapacity, Scale);
    }

    public void Add_RateOfFire_Modifier(float Scale)
    {
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.RateOfFire, Scale);
    }

    public void Add_ReloadSpeed_Modifier(float Scale)
    {
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.ReloadSpeed, Scale);
    }

    public void Add_ProjectileSpeed_Modifier(float Scale)
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.ProjectileSpeed, Scale);
    }

    public void Add_Durability_Modifier(float Scale)
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.Durability, Scale);
    }

    public void Add_Accuracy_Modifier(float Scale)
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.Accuracy, Scale);
    }

    public void Add_Penetration_Modifier(float Scale)
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.Penetrating, Scale);
    }
    
    public void Add_TriBarrel_Modifier()
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.TriBarrel, 1.0f);
    }
    
    public void Add_QuadBarrel_Modifier()
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.QuadBarrel, 1.0f);
    }

    public void Add_MultiTap_Modifier()
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.MultiTap, 1.0f);
    }

    public void Add_Suicidal_Modifier()
    {    
        Add_Modifier(Item_Upgrade_Tree_Node_Modifier_Type.Suicidal, 1.0f);
    }
}

[
	Name("Item_Upgrade_Tree"), 
	Description("Defines an upgrade tree for an item.") 
]
public native("Item_Upgrade_Tree") class Item_Upgrade_Tree 
{
    property Item_Upgrade_Tree_Node[] Nodes
	{
		public native("Get_Nodes") Item_Upgrade_Tree_Node[] Get();
		public native("Set_Nodes") void Set(Item_Upgrade_Tree_Node[] value);	
	}
    
	public native("Create") Item_Upgrade_Tree();

    public void Init()
    {
        Nodes = new Item_Upgrade_Tree_Node[0];
    }

	public static native("Find_By_Type") Item_Upgrade_Tree Find_By_Type(Type type_id);
	
    public native("Get_Active_Compound_Modifiers") float[] Get_Active_Compound_Modifiers(int[] upgrade_ids);

    public Item_Upgrade_Tree_Node Add_Node(int x, int y, bool bConnectUp, bool bConnectDown, bool bConnectLeft, bool bConnectRight, Type gemType, string name, int cost = 0)
    {
         if (this.Nodes == null)
         { 
             Init();
         }

         Item_Upgrade_Tree_Node node = new Item_Upgrade_Tree_Node();   
         node.Init();
         node.Position = Vec2(x, y);
         node.Name = name;
         node.Gem_Type = gemType;
         node.Connect_Up = bConnectUp;
         node.Connect_Down = bConnectDown;
         node.Connect_Left = bConnectLeft;
         node.Connect_Right = bConnectRight;
         node.Cost = cost;
         this.Nodes.AddLast(node);

         return node;
    }
}
