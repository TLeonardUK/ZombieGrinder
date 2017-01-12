using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.IO;

namespace Editor.Editors
{
    public partial class UpgradeTreeDBEditor : Form
    {
        private string m_path = "";
        private List<UpgradeTreeDBItem> m_items = new List<UpgradeTreeDBItem>();
        private bool m_state_updating = false;

        private const int CELL_PADDING = 8;
        private const int CELL_WIDTH = 48;
        private const int CELL_HEIGHT = 48;

        private int m_hover_cell_x = -1;
        private int m_hover_cell_y = -1;

        private int m_selected_cell_x = -1;
        private int m_selected_cell_y = -1;

        private UpgradeTreeDBItemGraphNode m_selected_node = null;

        public UpgradeTreeDBEditor(string path)
        {
            m_path = path;

            InitializeComponent();

            relativeComboBox.SelectedIndex = 0;

            LoadDB();
            UpdateItemList();
            UpdateState();
        }

        private void SaveDB()
        {
            XmlDocument doc = new XmlDocument();
            try
            {
                XmlElement xmlElement = doc.CreateElement("xml");
                XmlElement editorElement = doc.CreateElement("editor");
                XmlElement assetTypeElement = doc.CreateElement("asset_type");

                assetTypeElement.InnerText = "UpgradeTreeDB";

                XmlElement itemsElement = doc.CreateElement("items");

                foreach (UpgradeTreeDBItem item in m_items)
                {
                    XmlElement itemElement = doc.CreateElement("item");
                    itemElement.SetAttribute("name", item.Name);

                    foreach (UpgradeTreeItemVariant var in item.Variants)
                    {
                        XmlElement variantElement = doc.CreateElement("variant");
                        variantElement.SetAttribute("type", Enum.GetName(typeof(UpgradeTreeItemVariant), var));

                        itemElement.AppendChild(variantElement);
                    }

                    foreach (UpgradeTreeDBItemGraph var in item.Graphs)
                    {
                        XmlElement variantElement = doc.CreateElement("graph");
                        variantElement.SetAttribute("variant", Enum.GetName(typeof(UpgradeTreeItemVariant), var.Varient));
                        variantElement.SetAttribute("unique_id_counter", var.unique_id_counter.ToString());
                        variantElement.SetAttribute("shared_name", var.Shared_Name);

                        foreach (UpgradeTreeDBItemGraphNode node in var.Nodes)
                        {
                            XmlElement nodeElement = doc.CreateElement("node");
                            nodeElement.SetAttribute("x", node.x.ToString());
                            nodeElement.SetAttribute("y", node.y.ToString());
                            nodeElement.SetAttribute("connect_up", node.connect_up.ToString());
                            nodeElement.SetAttribute("connect_down", node.connect_down.ToString());
                            nodeElement.SetAttribute("connect_left", node.connect_left.ToString());
                            nodeElement.SetAttribute("connect_right", node.connect_right.ToString());    
                            nodeElement.SetAttribute("type", Enum.GetName(typeof(UpgradeTreeNodeType), node.type));
                            nodeElement.SetAttribute("unique_id", node.unique_id.ToString());
                            nodeElement.SetAttribute("cost", node.cost.ToString());

                            for (int i = 0; i < (int)UpgradeTreeItemModifier.COUNT; i++)
                            {
                                nodeElement.SetAttribute(Enum.GetName(typeof(UpgradeTreeItemModifier), (UpgradeTreeItemModifier)i), node.modifiers[i].ToString());
                            }
                                              
                            variantElement.AppendChild(nodeElement);
                        }

                        itemElement.AppendChild(variantElement);
                    }

                    itemsElement.AppendChild(itemElement);
                }
                
                editorElement.AppendChild(assetTypeElement);
                xmlElement.AppendChild(itemsElement);
                xmlElement.AppendChild(editorElement);
                doc.AppendChild(xmlElement);
                
                doc.Save(m_path);
            }
            catch (XmlException)
            {
                MessageBox.Show("Unable to save XML file.", "Save Fail", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
        }

        private void LoadDB()
        {
            XmlDocument doc = new XmlDocument();
            try
            {
                doc.Load(m_path);

                m_items.Clear();

                foreach (XmlElement element in doc.DocumentElement["items"].GetElementsByTagName("item"))
                {
                    UpgradeTreeDBItem item = new UpgradeTreeDBItem();
                    item.Name = element.GetAttribute("name");


                    foreach (XmlElement varElement in element.GetElementsByTagName("variant"))
                    {
                        UpgradeTreeItemVariant var = (UpgradeTreeItemVariant)Enum.Parse(typeof(UpgradeTreeItemVariant), varElement.GetAttribute("type"));
                        item.Variants.Add(var);
                    }

                    foreach (XmlElement varElement in element.GetElementsByTagName("graph"))
                    {
                        UpgradeTreeDBItemGraph graph = new UpgradeTreeDBItemGraph();
                        item.Graphs.Add(graph);

                        graph.Varient = (UpgradeTreeItemVariant)Enum.Parse(typeof(UpgradeTreeItemVariant), varElement.GetAttribute("variant"));
                        graph.unique_id_counter = int.Parse(varElement.GetAttribute("unique_id_counter"));
                        
                        if (varElement.HasAttribute("shared_name"))
                        {
                            graph.Shared_Name = varElement.GetAttribute("shared_name");
                        }

                        foreach (XmlElement nodeElement in varElement.GetElementsByTagName("node"))
                        {
                            UpgradeTreeDBItemGraphNode node = new UpgradeTreeDBItemGraphNode();
                            graph.Nodes.Add(node);

                            node.x = int.Parse(nodeElement.GetAttribute("x"));
                            node.y = int.Parse(nodeElement.GetAttribute("y"));
                            node.type = (UpgradeTreeNodeType)Enum.Parse(typeof(UpgradeTreeNodeType), nodeElement.GetAttribute("type"));
                            node.connect_up = bool.Parse(nodeElement.GetAttribute("connect_up"));
                            node.connect_down = bool.Parse(nodeElement.GetAttribute("connect_down"));
                            node.connect_left = bool.Parse(nodeElement.GetAttribute("connect_left"));
                            node.connect_right = bool.Parse(nodeElement.GetAttribute("connect_right"));
                            node.unique_id = int.Parse(nodeElement.GetAttribute("unique_id"));

                            if (nodeElement.HasAttribute("cost"))
                            {
                                node.cost = int.Parse(nodeElement.GetAttribute("cost"));
                            }

                            for (int i = 0; i < (int)UpgradeTreeItemModifier.COUNT; i++)
                            {
                                string name = Enum.GetName(typeof(UpgradeTreeItemModifier), (UpgradeTreeItemModifier)i);
                                if (nodeElement.HasAttribute(name))
                                {
                                    node.modifiers[i] = float.Parse(nodeElement.GetAttribute(name));
                                }
                                else
                                {
                                    node.modifiers[i] = 0.0f;
                                }
                            }                                     
                        }
                    }

                    m_items.Add(item);
                }

            }
            catch (XmlException)
            {
                MessageBox.Show("Unable to load XML file, it appears to be invalid.", "Load Fail", MessageBoxButtons.OK, MessageBoxIcon.Error);
                Close();
            }
        }

        private void ExportScript(string path)
        {
            TextWriter writer = File.CreateText(path);

            string[] VariantPrefixes = new string[(int)UpgradeTreeItemVariant.COUNT] {
                "",
                "Damage",
                "Ammo_Capacity",
                "Rate_Of_Fire",
                "Reload_Speed",
                "Projectile_Speed",
                "Accuracy",
                "Penetration",
                "Tri_Barrel",
                "Quad_Barrel",
                "Multi_Tap",
                "Suicidal"
            };
            string[] VarientPrefixLocaleNames = new string[(int)UpgradeTreeItemVariant.COUNT] {
                "",
                "#item_varient_damage",
                "#item_varient_ammo_capacity",
                "#item_varient_rate_of_fire",
                "#item_varient_reload_speed",
                "#item_varient_projectile_speed",
                "#item_varient_accuracy",
                "#item_varient_penetrating",
                "#item_varient_tri_barrel",
                "#item_varient_quad_barrel",
                "#item_varient_multi_tap",
                "#item_varient_suicidal"
            };

            writer.WriteLine("// -----------------------------------------------------------------------------");
            writer.WriteLine("//  Copyright (C) 2013-2014 TwinDrills, All Rights Reserved");
            writer.WriteLine("// -----------------------------------------------------------------------------");
            writer.WriteLine("//  WARNING: This file is automatically generated. Do not manually editor.");
            writer.WriteLine("// -----------------------------------------------------------------------------");
            writer.WriteLine("using runtime.math;");
            writer.WriteLine("using runtime.log;");
            writer.WriteLine("using game.items.item_archetype;");
            writer.WriteLine();

            foreach (UpgradeTreeDBItem item in m_items)
            {
                // Ensure graph exists for each varient.
                foreach (UpgradeTreeItemVariant var in item.Variants)
                {
                    item.Get_Or_Create_Graph(var);
                }

                writer.WriteLine("// -----------------------------------------------------------------------------");
                writer.WriteLine("//  Weapon - " + item.Name);
                writer.WriteLine("// -----------------------------------------------------------------------------");
                writer.WriteLine();

                // Dump each graph.
                foreach (UpgradeTreeDBItemGraph graph in item.Graphs)
                {
                    string class_name = "Item_"+item.Name+"";
                    string class_name_base = "Item_"+item.Name+"";
                    if (graph.Varient != UpgradeTreeItemVariant.Base)
                    {
                        class_name = "Item_"+item.Name+"_"+VariantPrefixes[(int)graph.Varient]+""; ;
                    }

                    // Work out the minimum values of the node positions are rebase the origin to 0.
                    int shift_x = 99999;
                    int shift_y = 99999;
                    foreach (UpgradeTreeDBItemGraphNode node in graph.Nodes)
                    {
                        if (node.x < shift_x)
                        {
                            shift_x = node.x;
                        }
                        if (node.y < shift_y)
                        {
                            shift_y = node.y;
                        }
                    }

                    if (graph.Shared_Name == "")
                    {
                        // Write out upgrade tree.
                        writer.WriteLine("public class " + class_name + "_Upgrade_Tree : Item_Upgrade_Tree");
                        writer.WriteLine("{");
                        writer.WriteLine("  " + class_name + "_Upgrade_Tree()");
                        writer.WriteLine("  {");
                        writer.WriteLine("      Item_Upgrade_Tree_Node node;");

                        // If no nodes, just add a root node so it actually works in game.
                        if (graph.Nodes.Count == 0)
                        {
                            writer.WriteLine();
                            writer.WriteLine("      node = Add_Node(0, 0, false, false, false, false, null, \"ROOT\");");
                        }

                        foreach (UpgradeTreeDBItemGraphNode node in graph.Nodes)
                        {
                            bool connect_up = node.connect_up;
                            bool connect_down = node.connect_down;
                            bool connect_left = node.connect_left;
                            bool connect_right = node.connect_right;

                            UpgradeTreeDBItemGraphNode up_node = graph.Get_Node(node.x, node.y - 1);
                            UpgradeTreeDBItemGraphNode down_node = graph.Get_Node(node.x, node.y + 1);
                            UpgradeTreeDBItemGraphNode left_node = graph.Get_Node(node.x - 1, node.y);
                            UpgradeTreeDBItemGraphNode right_node = graph.Get_Node(node.x + 1, node.y);

                            if (up_node != null && up_node.connect_down)
                            {
                                connect_up = true;
                            }

                            if (down_node != null && down_node.connect_up)
                            {
                                connect_down = true;
                            }

                            if (left_node != null && left_node.connect_right)
                            {
                                connect_left = true;
                            }

                            if (right_node != null && right_node.connect_left)
                            {
                                connect_right = true;
                            }

                            // Add node.
                            writer.WriteLine();
                            writer.WriteLine("      node = Add_Node({0}, {1}, {2}, {3}, {4}, {5}, {6}, \"NODE_{7}\", {8});",
                                node.x - shift_x,
                                node.y - shift_y,
                                connect_up ? "true" : "false",
                                connect_down ? "true" : "false",
                                connect_left ? "true" : "false",
                                connect_right ? "true" : "false",
                                node.type == UpgradeTreeNodeType.Root ? "null" : "typeof(Item_" + Enum.GetName(typeof(UpgradeTreeNodeType), node.type) + ")",
                                node.unique_id,
                                node.cost);

                            // Add modifiers.
                            if (node.Modifier_TriBarrel)
                            {
                                writer.WriteLine("      node.Add_TriBarrel_Modifier();");
                            }
                            if (node.Modifier_QuadBarrel)
                            {
                                writer.WriteLine("      node.Add_QuadBarrel_Modifier();");
                            }
                            if (node.Modifier_Tapping)
                            {
                                writer.WriteLine("      node.Add_MultiTap_Modifier();");
                            }
                            if (node.Modifier_Suicidal)
                            {
                                writer.WriteLine("      node.Add_Suicidal_Modifier();");
                            }
                            if (node.Modifier_Damage != 0.0f)
                            {
                                writer.WriteLine("      node.Add_Damage_Modifier({0});", node.Modifier_Damage);
                            }
                            if (node.Modifier_AmmoCapacity != 0.0f)
                            {
                                writer.WriteLine("      node.Add_AmmoCapacity_Modifier({0});", node.Modifier_AmmoCapacity);
                            }
                            if (node.Modifier_RateOfFire != 0.0f)
                            {
                                writer.WriteLine("      node.Add_RateOfFire_Modifier({0});", node.Modifier_RateOfFire);
                            }
                            if (node.Modifier_ReloadSpeed != 0.0f)
                            {
                                writer.WriteLine("      node.Add_ReloadSpeed_Modifier({0});", node.Modifier_ReloadSpeed);
                            }
                            if (node.Modifier_ProjectileSpeed != 0.0f)
                            {
                                writer.WriteLine("      node.Add_ProjectileSpeed_Modifier({0});", node.Modifier_ProjectileSpeed);
                            }
                            if (node.Modifier_Accuracy != 0.0f)
                            {
                                writer.WriteLine("      node.Add_Accuracy_Modifier({0});", node.Modifier_Accuracy);
                            }
                            if (node.Modifier_Penetration != 0.0f)
                            {
                                writer.WriteLine("      node.Add_Penetration_Modifier({0});", node.Modifier_Penetration);
                            }
                            if (node.Modifier_Durability != 0.0f)
                            {
                                writer.WriteLine("      node.Add_Durability_Modifier({0});", node.Modifier_Durability);
                            }
                        }

                        writer.WriteLine("  }");
                        writer.WriteLine("}");
                        writer.WriteLine();
                    }
                    else
                    {
                        string shared_name = "Item_"+graph.Shared_Name+"";
                        if (graph.Varient != UpgradeTreeItemVariant.Base)
                        {
                            shared_name = "Item_"+graph.Shared_Name+"_"+VariantPrefixes[(int)graph.Varient]+""; ;
                        }

                        // Write out upgrade tree.
                        writer.WriteLine("public class " + class_name + "_Upgrade_Tree : " + shared_name + "_Upgrade_Tree");
                        writer.WriteLine("{");
                        writer.WriteLine("}");
                    }

                    // Write out varient item if required.
                    if (graph.Varient != UpgradeTreeItemVariant.Base)
                    {
                        writer.WriteLine("public class " + class_name + " : Item_" + item.Name);
                        writer.WriteLine("{");
                        writer.WriteLine("  " + class_name + "()");
                        writer.WriteLine("  {");
                        writer.WriteLine("      Varient_Prefix = \"{0}\";", VarientPrefixLocaleNames[(int)graph.Varient]);
                        writer.WriteLine("      Override_Inventory_ID_Name = \"{0}\";", graph.Varient == UpgradeTreeItemVariant.RateOfFire ? class_name_base : "");
                        writer.WriteLine("      Is_Buyable = false;");
                        writer.WriteLine("      Is_Dropable = false;");
                        writer.WriteLine("      Is_Inventory_Droppable = true;");
                        switch (graph.Varient)
                        {
                            case UpgradeTreeItemVariant.Damage:
                                writer.WriteLine("      Rarity = Item_Rarity.Rare;");
                                break;
                            case UpgradeTreeItemVariant.AmmoCapacity:
                                writer.WriteLine("      Rarity = Item_Rarity.Uncommon;");
                                break;
                            case UpgradeTreeItemVariant.RateOfFire:
                                writer.WriteLine("      Rarity = Item_Rarity.Rare;");
                                break;
                            case UpgradeTreeItemVariant.ReloadSpeed:
                                writer.WriteLine("      Rarity = Item_Rarity.Uncommon;");
                                break;
                            case UpgradeTreeItemVariant.ProjectileSpeed:
                                writer.WriteLine("      Rarity = Item_Rarity.Uncommon;");
                                break;
                            case UpgradeTreeItemVariant.Accuracy:
                                writer.WriteLine("      Rarity = Item_Rarity.Uncommon;");
                                break;
                            case UpgradeTreeItemVariant.Penetration:
                                writer.WriteLine("      Rarity = Item_Rarity.Uncommon;");
                                break;
                            case UpgradeTreeItemVariant.TriBarrel:
                                writer.WriteLine("      Rarity = Item_Rarity.Ultra_Rare;");
                                break;
                            case UpgradeTreeItemVariant.QuadBarrel:
                                writer.WriteLine("      Rarity = Item_Rarity.Legendary;");
                                break;
                            case UpgradeTreeItemVariant.Tapping:
                                writer.WriteLine("      Rarity = Item_Rarity.Ultra_Rare;");
                                break;
                            case UpgradeTreeItemVariant.Suicidal:
                                writer.WriteLine("      Rarity = Item_Rarity.Legendary;");
                                break;
                            default:
                                writer.WriteLine("      Rarity = Item_Rarity.Common;");
                                break;
                        }
                        writer.WriteLine("      Upgrade_Tree = Item_Upgrade_Tree.Find_By_Type(typeof("+class_name+"_Upgrade_Tree));");
                        writer.WriteLine("  }");
                        writer.WriteLine("}");
                        writer.WriteLine();
                    }
                }
            }
            
            writer.Close();
        }

        private void UpdateState()
        {
            UpgradeTreeItemTag tag = (ItemTreeView.SelectedNode != null) ? (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag : null;

            UpgradeTreeDBItemGraph graph = Get_Graph();

            m_state_updating = true;

            DeleteItemButton.Enabled = (tag != null);
            NameTextBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            SharedTextBox.Enabled = (graph != null);

            DamageVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            AmmoCapacityVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            RateOfFireVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            ReloadSpeedVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            ProjectileSpeedVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            AccuracyVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            PenetrationVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            TriBarrelVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            QuadBarrelVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            TappingVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);
            SuicidalVariantCheckBox.Enabled = (tag != null && tag.Varient == UpgradeTreeItemVariant.Base);

            NameTextBox.Text = tag != null ? tag.Item.Name : "";
            SharedTextBox.Text = graph != null ? graph.Shared_Name : "";
            DamageVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.Damage));
            AmmoCapacityVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.AmmoCapacity));
            RateOfFireVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.RateOfFire));
            ReloadSpeedVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.ReloadSpeed));
            ProjectileSpeedVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.ProjectileSpeed));
            AccuracyVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.Accuracy));
            PenetrationVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.Penetration));
            TriBarrelVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.TriBarrel));
            QuadBarrelVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.QuadBarrel));
            TappingVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.Tapping));
            SuicidalVariantCheckBox.Checked = (tag != null && tag.Item.Variants.Contains(UpgradeTreeItemVariant.Suicidal));

            m_state_updating = false;
        }

        private void UpdateItemList()
        {
            ItemTreeView.Nodes.Clear();
            foreach (UpgradeTreeDBItem item in m_items)
            {
                TreeNode node = ItemTreeView.Nodes.Add(item.Name);
                node.Tag = new UpgradeTreeItemTag(item, UpgradeTreeItemVariant.Base, node);

                UpdateVariantNodes(node, item);
            }
        }

        private void UpdateVariantNodes(TreeNode Node, UpgradeTreeDBItem Item)
        {
            Node.Nodes.Clear();

            foreach (UpgradeTreeItemVariant var in Item.Variants)
            {
                if (var == UpgradeTreeItemVariant.Base)
                {
                    continue;
                }

                TreeNode subNode = Node.Nodes.Add(Enum.GetName(typeof(UpgradeTreeItemVariant), var) + " Variant");
                subNode.Tag = new UpgradeTreeItemTag(Item, var, Node);
            }
        }

        private void CreateItemButton_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItem item = new UpgradeTreeDBItem();
            item.Name = "Untitled Item";
            item.Variants.Add(UpgradeTreeItemVariant.Base);
            m_items.Add(item);

            UpdateItemList();
        }

        private void DeleteItemButton_Click(object sender, EventArgs e)
        {
            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            m_items.Remove(tag.Item);

            UpdateItemList();   
        }

        private void ItemTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            m_selected_node = null;
            NodePropertyGrid.SelectedObject = null;

            UpdateState();
            GraphPanel.Invalidate();
        }

        private void SaveButton_Click(object sender, EventArgs e)
        {
            SaveDB();
        }

        private void ToggleVariant(UpgradeTreeDBItem item, UpgradeTreeItemVariant var)
        {
            if (item.Variants.Contains(var))
            {
                item.Variants.Remove(var);
            }
            else
            {
                item.Variants.Add(var);
            }
        }

        private void NameTextBox_TextChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            tag.Item.Name = NameTextBox.Text;
            ItemTreeView.SelectedNode.Text = tag.Item.Name;
        }

        private void SharedTextBox_TextChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeDBItemGraph graph = Get_Graph();
            graph.Shared_Name = SharedTextBox.Text;
        }

        private void DamageVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.Damage);
            UpdateVariantNodes(tag.Node, tag.Item);                           
        }

        private void AmmoCapacityVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.AmmoCapacity);
            UpdateVariantNodes(tag.Node, tag.Item);                    
        }

        private void RateOfFireVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.RateOfFire);
            UpdateVariantNodes(tag.Node, tag.Item);                  
        }

        private void ReloadSpeedVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.ReloadSpeed);
            UpdateVariantNodes(tag.Node, tag.Item);                  
        }

        private void ProjectileSpeedVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.ProjectileSpeed);
            UpdateVariantNodes(tag.Node, tag.Item);                    
        }

        private void AccuracyVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.Accuracy);
            UpdateVariantNodes(tag.Node, tag.Item);                        
        }

        private void PenetrationVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.Penetration);
            UpdateVariantNodes(tag.Node, tag.Item);                      
        }

        private void TriBarrelVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.TriBarrel);
            UpdateVariantNodes(tag.Node, tag.Item);                        
        }

        private void QuadBarrelVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.QuadBarrel);
            UpdateVariantNodes(tag.Node, tag.Item);                      
        }

        private void TappingVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.Tapping);
            UpdateVariantNodes(tag.Node, tag.Item);                         
        }

        private void SuicidalVariantCheckBox_CheckedChanged(object sender, EventArgs e)
        {
            if (m_state_updating || ItemTreeView.SelectedNode == null)
            {
                return;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            ToggleVariant(tag.Item, UpgradeTreeItemVariant.Suicidal);
            UpdateVariantNodes(tag.Node, tag.Item);              
        }

        private void GraphPanel_Paint(object sender, PaintEventArgs e)
        {
            Pen bluePen = new Pen(Color.Blue, 2);
            Pen connectPen = new Pen(Color.Yellow, 4);
            Pen selectedPen = new Pen(Color.CornflowerBlue, 2);
            Pen blackPen = new Pen(Color.Black, 1);
            blackPen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;

            Brush bgRect = new SolidBrush(Color.White);
            Brush selectedBrush = new HatchBrush(HatchStyle.ForwardDiagonal, Color.CornflowerBlue, Color.Transparent);

            int gridWidth = (int)(e.Graphics.VisibleClipBounds.Width / (CELL_WIDTH + CELL_PADDING)) + 1;
            int gridHeight = (int)(e.Graphics.VisibleClipBounds.Height / (CELL_HEIGHT + CELL_PADDING)) + 1;
            
            UpgradeTreeDBItemGraph graph = Get_Graph();

            if (graph != null)
            {
                for (int x = 0; x < gridWidth; x++)
                {
                    for (int y = 0; y < gridHeight; y++)
                    {
                        Rectangle rect = new Rectangle(
                            CELL_PADDING + ((CELL_WIDTH + CELL_PADDING) * x),
                            CELL_PADDING + ((CELL_HEIGHT + CELL_PADDING) * y),
                            CELL_WIDTH,
                            CELL_HEIGHT
                        );
                        
                        Rectangle rect_left = new Rectangle(
                            CELL_PADDING + ((CELL_WIDTH + CELL_PADDING) * (x - 1)),
                            CELL_PADDING + ((CELL_HEIGHT + CELL_PADDING) * y),
                            CELL_WIDTH,
                            CELL_HEIGHT
                        );

                        Rectangle rect_right = new Rectangle(
                            CELL_PADDING + ((CELL_WIDTH + CELL_PADDING) * (x + 1)),
                            CELL_PADDING + ((CELL_HEIGHT + CELL_PADDING) * y),
                            CELL_WIDTH,
                            CELL_HEIGHT
                        );

                        Rectangle rect_up = new Rectangle(
                            CELL_PADDING + ((CELL_WIDTH + CELL_PADDING) * x),
                            CELL_PADDING + ((CELL_HEIGHT + CELL_PADDING) * (y - 1)),
                            CELL_WIDTH,
                            CELL_HEIGHT
                        );

                        Rectangle rect_down = new Rectangle(
                            CELL_PADDING + ((CELL_WIDTH + CELL_PADDING) * x),
                            CELL_PADDING + ((CELL_HEIGHT + CELL_PADDING) * (y + 1)),
                            CELL_WIDTH,
                            CELL_HEIGHT
                        );

                        UpgradeTreeDBItemGraphNode node = graph.Get_Node(x, y);
                        if (node != null)
                        {
                            e.Graphics.FillRectangle(bgRect, rect);

                            if (node.connect_down)
                            {
                                e.Graphics.DrawLine(connectPen, 
                                    new Point(rect.X + (rect.Width / 2), rect.Y + rect.Height), 
                                    new Point(rect_down.X + (rect_down.Width / 2), rect_down.Y)
                                );
                            }
                            if (node.connect_up)
                            {
                                e.Graphics.DrawLine(connectPen, 
                                    new Point(rect_up.X + (rect_up.Width / 2), rect_up.Y + rect_up.Height), 
                                    new Point(rect.X + (rect.Width / 2), rect.Y)
                                );
                            }
                            if (node.connect_left)
                            {
                                e.Graphics.DrawLine(connectPen, 
                                    new Point(rect_left.X + rect_left.Width, rect_left.Y + (rect_left.Height / 2)), 
                                    new Point(rect.X, rect.Y + (rect.Height / 2))
                                );
                            }
                            if (node.connect_right)
                            {
                                e.Graphics.DrawLine(connectPen, 
                                    new Point(rect_right.X, rect_right.Y + (rect_right.Height / 2)),
                                    new Point(rect.X + rect.Width, rect.Y + (rect.Height / 2))
                                );
                            }

                            Image gemImage = gemImageList.Images[(int)node.type];
                            Rectangle imageRect = rect;
                            imageRect.Inflate(-10, -10);
                            e.Graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
                            e.Graphics.DrawImage(gemImage, imageRect);
                        }

                        if (m_selected_node == node && m_selected_node != null)
                        {
                            e.Graphics.FillRectangle(selectedBrush, rect);
                            e.Graphics.DrawRectangle(selectedPen, rect);
                        }
                        else if (x == m_hover_cell_x && y == m_hover_cell_y)
                        {
                            e.Graphics.DrawRectangle(bluePen, rect);
                        }
                        else
                        {
                            e.Graphics.DrawRectangle(blackPen, rect);
                        }
                    }
                }
            }
        }

        private void UpgradeTreeDBEditor_Resize(object sender, EventArgs e)
        {
            GraphPanel.Invalidate();
        }

        private void GraphPanel_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                int cellX = (int)((e.X - CELL_PADDING) / (CELL_WIDTH + CELL_PADDING));
                int cellY = (int)((e.Y - CELL_PADDING) / (CELL_HEIGHT + CELL_PADDING));

                m_selected_cell_x = cellX;
                m_selected_cell_y = cellY;

                UpgradeTreeDBItemGraph graph = Get_Graph();
                if (graph.Get_Node(cellX, cellY) != null)
                {
                    connectUpToolStripMenuItem.Enabled = true;
                    connectDownToolStripMenuItem.Enabled = true;
                    connectLeftToolStripMenuItem.Enabled = true;
                    connectRightToolStripMenuItem.Enabled = true;
                    deleteNodeToolStripMenuItem.Enabled = true;
                }
                else
                {
                    connectUpToolStripMenuItem.Enabled = false;
                    connectDownToolStripMenuItem.Enabled = false;
                    connectLeftToolStripMenuItem.Enabled = false;
                    connectRightToolStripMenuItem.Enabled = false;
                    deleteNodeToolStripMenuItem.Enabled = false;
                }

                GridPanelAddContextMenu.Show(GraphPanel, e.X, e.Y);
            }
            else
            {
                int cellX = (int)((e.X - CELL_PADDING) / (CELL_WIDTH + CELL_PADDING));
                int cellY = (int)((e.Y - CELL_PADDING) / (CELL_HEIGHT + CELL_PADDING));

                m_selected_cell_x = cellX;
                m_selected_cell_y = cellY;

                UpgradeTreeDBItemGraph graph = Get_Graph();
                m_selected_node = graph.Get_Node(cellX, cellY);
                NodePropertyGrid.SelectedObject = m_selected_node;
            }
        }
    
        private void GraphPanel_MouseMove(object sender, MouseEventArgs e)
        {
            int cellX = (int)((e.X - CELL_PADDING) / (CELL_WIDTH + CELL_PADDING));
            int cellY = (int)((e.Y - CELL_PADDING) / (CELL_HEIGHT + CELL_PADDING));

            m_hover_cell_x = cellX;
            m_hover_cell_y = cellY;

            GraphPanel.Invalidate();
        }

        private UpgradeTreeDBItemGraph Get_Graph()
        {
            if (ItemTreeView.SelectedNode == null)
            {
                return null;
            }

            UpgradeTreeItemTag tag = (UpgradeTreeItemTag)ItemTreeView.SelectedNode.Tag;
            return tag.Item.Get_Or_Create_Graph(tag.Varient);
        }

        private void Add_Node(UpgradeTreeNodeType type, int x, int y)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();

            bool bIsNew = false;
            if (graph.Get_Node(x, y) == null)
            {
                bIsNew = true;
            }

            UpgradeTreeDBItemGraphNode node = graph.Get_Or_Create_Node(x, y);
            node.type = type;
            if (bIsNew)
            {
                node.unique_id = graph.unique_id_counter++;
            }
            GraphPanel.Invalidate();
        }

        private void Shift_Nodes(int x, int y)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            if (relativeComboBox.SelectedIndex == 0)
            {
                foreach (UpgradeTreeDBItemGraphNode node in graph.Nodes)
                {
                    node.x += x;
                    node.y += y;
                }
            }
            else if (m_selected_cell_x >= 0 && m_selected_cell_y >= 0)
            {
                foreach (UpgradeTreeDBItemGraphNode node in graph.Nodes)
                {
                    if (node.x >= m_selected_cell_x)
                        node.x += x;
                    if (node.y >= m_selected_cell_y)
                        node.y += y;
                }
            }
            GraphPanel.Invalidate();
        }

        private void greenLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Green_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void greenLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Green_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void greenLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Green_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void greenLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Green_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void greenLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Green_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void purpleLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Purple_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void purpleLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Purple_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void purpleLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Purple_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void purpleLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Purple_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void purpleLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Purple_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void yellowLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Yellow_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void yellowLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Yellow_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void yellowLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Yellow_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void yellowLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Yellow_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void yellowLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Yellow_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void orangeLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Orange_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void orangeLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Orange_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void orangeLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Orange_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void orangeLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Orange_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void orangeLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Orange_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void blueLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Blue_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void blueLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Blue_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void blueLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Blue_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void blueLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Blue_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void blueLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Blue_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void aquaLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Aqua_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void aquaLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Aqua_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void aquaLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Aqua_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void aquaLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Aqua_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void aquaLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Aqua_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void redLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Red_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }

        private void redLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Red_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void redLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Red_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void redLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Red_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void redLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Red_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void pinkLevel1ToolStripMenuItem_Click(object sender, EventArgs e)
        {            
            Add_Node(UpgradeTreeNodeType.Pink_Gem_1, m_selected_cell_x, m_selected_cell_y);
        }
        
        private void pinkLevel2ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Pink_Gem_2, m_selected_cell_x, m_selected_cell_y);
        }

        private void pinkLevel3ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Pink_Gem_3, m_selected_cell_x, m_selected_cell_y);
        }

        private void pinkLevel4ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Pink_Gem_4, m_selected_cell_x, m_selected_cell_y);
        }

        private void pinkLevel5ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Pink_Gem_5, m_selected_cell_x, m_selected_cell_y);
        }

        private void shiftUpButton_Click(object sender, EventArgs e)
        {
            Shift_Nodes(0, -1);
        }

        private void shiftLeftButton_Click(object sender, EventArgs e)
        {
            Shift_Nodes(-1, 0);
        }

        private void shiftRightButton_Click(object sender, EventArgs e)
        {
            Shift_Nodes(1, 0);
        }

        private void shiftDownButton_Click(object sender, EventArgs e)
        {
            Shift_Nodes(0, 1);
        }

        private void deleteNodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
            graph.Nodes.Remove(node);
        }
        
        private void UpgradeTreeDBEditor_KeyPress(object sender, KeyPressEventArgs e)
        {
        }

        private void UpgradeTreeDBEditor_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete)
            {
                UpgradeTreeDBItemGraph graph = Get_Graph();
                if (graph != null)
                {
                    UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
                    if (node != null)
                    {
                        graph.Nodes.Remove(node);
                    }
                }
            }
        }

        private void connectUpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
            node.connect_up = !node.connect_up;
        }

        private void connectDownToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
            node.connect_down = !node.connect_down;
        }

        private void connectLeftToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
            node.connect_left = !node.connect_left;
        }

        private void connectRightToolStripMenuItem_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            UpgradeTreeDBItemGraphNode node = graph.Get_Node(m_selected_cell_x, m_selected_cell_y);
            node.connect_right = !node.connect_right;
        }

        private void addRootToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Add_Node(UpgradeTreeNodeType.Root, m_selected_cell_x, m_selected_cell_y);
        }

        private void ExportButton_Click(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Title = "Export Script";
            dialog.Filter = "Script Files | *.xs";
            dialog.DefaultExt = "xs";
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                ExportScript(dialog.FileName);
            }
        }

        private UpgradeTreeDBItemGraph m_copy_graph = null;

        private void CopyButton_Click(object sender, EventArgs e)
        {
            PasteButton.Enabled = true;
            m_copy_graph = Get_Graph();
        }

        private void PasteButton_Click(object sender, EventArgs e)
        {
            UpgradeTreeDBItemGraph graph = Get_Graph();
            graph.unique_id_counter = m_copy_graph.unique_id_counter;
            graph.Nodes.Clear();

            foreach (UpgradeTreeDBItemGraphNode node in m_copy_graph.Nodes)
            {
                UpgradeTreeDBItemGraphNode new_node = node.Clone();
                graph.Nodes.Add(new_node);
            }
        }
    }

    public enum UpgradeTreeNodeType
    {
        Green_Gem_1,
        Green_Gem_2,
        Green_Gem_3,
        Green_Gem_4,
        Green_Gem_5,
        Purple_Gem_1,
        Purple_Gem_2,
        Purple_Gem_3,
        Purple_Gem_4,
        Purple_Gem_5,
        Yellow_Gem_1,
        Yellow_Gem_2,
        Yellow_Gem_3,
        Yellow_Gem_4,
        Yellow_Gem_5,
        Orange_Gem_1,
        Orange_Gem_2,
        Orange_Gem_3,
        Orange_Gem_4,
        Orange_Gem_5,
        Blue_Gem_1,
        Blue_Gem_2,
        Blue_Gem_3,
        Blue_Gem_4,
        Blue_Gem_5,
        Aqua_Gem_1,
        Aqua_Gem_2,
        Aqua_Gem_3,
        Aqua_Gem_4,
        Aqua_Gem_5,
        Red_Gem_1,
        Red_Gem_2,
        Red_Gem_3,
        Red_Gem_4,
        Red_Gem_5,
        Pink_Gem_1,
        Pink_Gem_2,
        Pink_Gem_3,
        Pink_Gem_4,
        Pink_Gem_5,
        Root
    }

    public enum UpgradeTreeItemVariant
    {
        Base,
        Damage,
        AmmoCapacity,
        RateOfFire,
        ReloadSpeed,
        ProjectileSpeed,
        Accuracy,
        Penetration,
        TriBarrel,
        QuadBarrel,
        Tapping,
        Suicidal,

        COUNT
    }

    public enum UpgradeTreeItemModifier
    {
        Damage,
        AmmoCapacity,
        RateOfFire,
        ReloadSpeed,
        ProjectileSpeed,
        Accuracy,
        Penetration,
        Durability,
        TriBarrel,
        QuadBarrel,
        Tapping,
        Suicidal,

        COUNT
    }

    public class UpgradeTreeDBItemGraphNode
    {
        [Browsable(true), Category("Modifiers"), DisplayName("Damage")]
        public float Modifier_Damage
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Damage]; }
            set { modifiers[(int)UpgradeTreeItemModifier.Damage] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Ammo Capacity")]
        public float Modifier_AmmoCapacity
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.AmmoCapacity]; }
            set { modifiers[(int)UpgradeTreeItemModifier.AmmoCapacity] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Rate of Fire")]
        public float Modifier_RateOfFire
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.RateOfFire]; }
            set { modifiers[(int)UpgradeTreeItemModifier.RateOfFire] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Reload Speed")]
        public float Modifier_ReloadSpeed
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.ReloadSpeed]; }
            set { modifiers[(int)UpgradeTreeItemModifier.ReloadSpeed] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Projectile Speed")]
        public float Modifier_ProjectileSpeed
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.ProjectileSpeed]; }
            set { modifiers[(int)UpgradeTreeItemModifier.ProjectileSpeed] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Accuracy")]
        public float Modifier_Accuracy
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Accuracy]; }
            set { modifiers[(int)UpgradeTreeItemModifier.Accuracy] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Penetration")]
        public float Modifier_Penetration
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Penetration]; }
            set { modifiers[(int)UpgradeTreeItemModifier.Penetration] = value; }
        }
        [Browsable(true), Category("Modifiers"), DisplayName("Durability")]
        public float Modifier_Durability
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Durability]; }
            set { modifiers[(int)UpgradeTreeItemModifier.Durability] = value; }
        }
        [Browsable(true), Category("Styles"), DisplayName("Tri-Barrel")]
        public bool Modifier_TriBarrel
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.TriBarrel] != 0.0f; }
            set { modifiers[(int)UpgradeTreeItemModifier.TriBarrel] = (value ? 1.0f : 0.0f); }
        }
        [Browsable(true), Category("Styles"), DisplayName("Quad-Barrel")]
        public bool Modifier_QuadBarrel
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.QuadBarrel] != 0.0f; }
            set { modifiers[(int)UpgradeTreeItemModifier.QuadBarrel] = (value ? 1.0f : 0.0f); }
        }
        [Browsable(true), Category("Styles"), DisplayName("Tapping")]
        public bool Modifier_Tapping
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Tapping] != 0.0f; }
            set { modifiers[(int)UpgradeTreeItemModifier.Tapping] = (value ? 1.0f : 0.0f); }
        }
        [Browsable(true), Category("Styles"), DisplayName("Suicidal")]
        public bool Modifier_Suicidal
        {
            get { return modifiers[(int)UpgradeTreeItemModifier.Suicidal] != 0.0f; }
            set { modifiers[(int)UpgradeTreeItemModifier.Suicidal] = (value ? 1.0f : 0.0f); }
        }
        [Browsable(true), Category("Costs"), DisplayName("Cost")]
        public int Cost
        {
            get { return cost; }
            set { cost = value; }
        }
        
        public float[] modifiers = new float[(int)UpgradeTreeItemModifier.COUNT];
        public int x = 0;
        public int y = 0;
        public int unique_id = 0;
        public int cost = 1;
        public bool connect_up = false;
        public bool connect_down = false;
        public bool connect_left = false;
        public bool connect_right = false;
        public UpgradeTreeNodeType type = UpgradeTreeNodeType.Green_Gem_1;

        public UpgradeTreeDBItemGraphNode Clone()
        {
            UpgradeTreeDBItemGraphNode clone = (UpgradeTreeDBItemGraphNode)this.MemberwiseClone();
            clone.modifiers = (float[])modifiers.Clone();
            return clone;
        }
    }

    public class UpgradeTreeDBItemGraph
    {
        public UpgradeTreeItemVariant Varient = UpgradeTreeItemVariant.Base;
        public List<UpgradeTreeDBItemGraphNode> Nodes = new List<UpgradeTreeDBItemGraphNode>();

        public string Shared_Name = "";

        public int unique_id_counter = 0;

        public UpgradeTreeDBItemGraphNode Get_Or_Create_Node(int x, int y)
        {
            foreach (UpgradeTreeDBItemGraphNode node in Nodes)
            {
                if (node.x == x && node.y == y)
                {
                    return node;
                }
            }

            UpgradeTreeDBItemGraphNode newNode = new UpgradeTreeDBItemGraphNode();
            newNode.x = x;
            newNode.y = y;
            Nodes.Add(newNode);

            return newNode;
        }

        public UpgradeTreeDBItemGraphNode Get_Node(int x, int y)
        {
            foreach (UpgradeTreeDBItemGraphNode node in Nodes)
            {
                if (node.x == x && node.y == y)
                {
                    return node;
                }
            }

            return null;
        }
    }

    public class UpgradeTreeDBItem
    {
        public string Name;
        public List<UpgradeTreeItemVariant> Variants = new List<UpgradeTreeItemVariant>();
        public List<UpgradeTreeDBItemGraph> Graphs = new List<UpgradeTreeDBItemGraph>();

        public UpgradeTreeDBItemGraph Get_Or_Create_Graph(UpgradeTreeItemVariant var)
        {
            foreach (UpgradeTreeDBItemGraph gr in Graphs)
            {
                if (gr.Varient == var)
                {
                    return gr;
                }
            }

            UpgradeTreeDBItemGraph graph = new UpgradeTreeDBItemGraph();
            graph.Varient = var;
            Graphs.Add(graph);

            return graph;
        }
    }

    public class UpgradeTreeItemTag
    {
        public UpgradeTreeDBItem Item;
        public UpgradeTreeItemVariant Varient;
        public TreeNode Node;

        public UpgradeTreeItemTag(UpgradeTreeDBItem item, UpgradeTreeItemVariant var, TreeNode node)
        {
            Item = item;
            Varient = var;
            Node = node;
        }
    }
}
