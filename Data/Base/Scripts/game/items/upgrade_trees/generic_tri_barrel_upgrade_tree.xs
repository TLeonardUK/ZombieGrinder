// -----------------------------------------------------------------------------
// 	Copyright (C) 2013-2014 TwinDrills, All Rights Reserved
// -----------------------------------------------------------------------------
using game.items.item_archetype;

public class Item_Generic_Tri_Barrel_Upgrade_Tree : Item_Upgrade_Tree
{
    Item_Generic_Tri_Barrel_Upgrade_Tree()
    {
        Item_Upgrade_Tree_Node node;

        node = Add_Node(0, 0, false, false, false, true, null, "ROOT");
                
        node = Add_Node(1, 0, false, false, true, false, typeof(Item_Red_Gem_5), "ROF_TIER_4");        
        node.Add_TriBarrel_Modifier();
    }
}