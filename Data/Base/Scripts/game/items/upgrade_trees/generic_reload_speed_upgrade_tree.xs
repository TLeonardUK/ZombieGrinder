// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.items.item_archetype;

public class Item_Generic_Reload_Speed_Upgrade_Tree : Item_Upgrade_Tree
{
    Item_Generic_Reload_Speed_Upgrade_Tree()
    {
        Item_Upgrade_Tree_Node node;

        node = Add_Node(0, 0, false, false, false, true, null, "ROOT");
        
        node = Add_Node(1, 0, false, false, true, false, typeof(Item_Red_Gem_5), "TIER_1");        
        node.Add_ReloadSpeed_Modifier(0.20f);

        node = Add_Node(2, 0, false, false, true, false, typeof(Item_Red_Gem_5), "TIER_2");        
        node.Add_ReloadSpeed_Modifier(0.20f);

        node = Add_Node(3, 0, false, false, true, false, typeof(Item_Red_Gem_5), "TIER_3");        
        node.Add_ReloadSpeed_Modifier(0.20f);
    }
}