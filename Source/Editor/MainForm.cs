using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using Editor.Editors;

namespace Editor
{
    public partial class MainForm : Form
    {
        private string m_CurrentModPath = "";
        private string m_CurrentModName = "";

        private const int DEFAULT_IMAGE_LIST_SIZE = 3;

        public MainForm()
        {
            InitializeComponent();

            TryOpenMod(Environment.CurrentDirectory + "\\..\\Data\\Base");
        }

        private bool TryOpenMod(string path)
        {
            string mod_name = Path.GetFileName(path);
            string xml_path = path + "\\" + mod_name + ".xml";

            if (!File.Exists(xml_path))
            {
                return false;
            }

            m_CurrentModPath = Path.GetFullPath(path);
            m_CurrentModName = mod_name;

            ModNameLabel.Text = "Mod: " + mod_name;

            PopulateAssetTree();
            PopulateItemList();

            return true;
        }
         
        private void OpenProjectButton_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.SelectedPath = Path.GetFullPath(Environment.CurrentDirectory + "\\..\\Data");
            dialog.Description = "Select Mod Folder";
            dialog.ShowNewFolderButton = false;
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                if (!TryOpenMod(dialog.SelectedPath))
                {
                    MessageBox.Show("Failed to open mod. Folder does not appear to be formatted correctly for a mod.", "Failed", MessageBoxButtons.OK, MessageBoxIcon.Error); 
                }
            }
        }

        private void ExitButton_Click(object sender, EventArgs e)
        {
            Environment.Exit(0);
        }

        private void PopulateAssetTree()
        {
            AssetTree.Nodes.Clear();

            TreeNode RootNode = AssetTree.Nodes.Add(m_CurrentModPath, m_CurrentModName, 1, 1);
            RootNode.Tag = m_CurrentModPath;
            PopulateAssetTreeFolder_r(m_CurrentModPath, RootNode);

            AssetTree.SelectedNode = RootNode;
        }

        private void PopulateAssetTreeFolder_r(string path, TreeNode parentNode)
        {   
            foreach (string dir in Directory.EnumerateDirectories(path))
            {                    
                string fullPath = Path.GetFullPath(dir);
                string folderName = Path.GetFileName(dir);

                TreeNode node = parentNode.Nodes.Add(fullPath, folderName, 0, 0);
                node.Tag = fullPath;
                PopulateAssetTreeFolder_r(fullPath, node);   
            }
        }
        
        private void PopulateItemList()
        {
            ItemListView.Items.Clear();

            if (AssetTree.SelectedNode == null) 
            {
                return;
            }

            while (folderImageList.Images.Count > DEFAULT_IMAGE_LIST_SIZE)
            {
                folderImageList.Images.RemoveAt(folderImageList.Images.Count - 1);
            }

            string dirPath = AssetTree.SelectedNode.Tag.ToString();

            foreach (string file in Directory.EnumerateDirectories(dirPath))
            {
                string fullPath = Path.GetFullPath(file);
                string fileName = Path.GetFileName(file);
                
                ListViewItem item = ItemListView.Items.Add(fullPath, "Folder", 0);
                item.SubItems.Add("");
                item.SubItems.Add(fileName);
                item.Tag = fullPath;
            }
            
            foreach (string file in Directory.EnumerateFiles(dirPath))
            {
                string fullPath = Path.GetFullPath(file);
                string fileName = Path.GetFileName(file);

                AssetType assetType = AssetHelper.GetAssetType(fullPath);
                string assetTypeDescription = AssetHelper.GetAssetTypeName(assetType);
                string fileSize = ((int)new FileInfo(fullPath).Length).BytesToHumanReadable();

                int imageIndex = 2;
                Image thumbnail = WindowsThumbnailProvider.GetThumbnail(fullPath, 16, 16, ThumbnailOptions.None);
                if (thumbnail != null)
                {
                    folderImageList.Images.Add(thumbnail);
                    imageIndex = folderImageList.Images.Count - 1;
                }

                ListViewItem item = ItemListView.Items.Add(fullPath, assetTypeDescription, imageIndex);
                item.SubItems.Add(fileSize);
                item.SubItems.Add(fileName);
                item.Tag = fullPath;
            }

            string relativeDir = m_CurrentModName + dirPath.Substring(m_CurrentModPath.Length);

            AssetPathLabel.Text = "Path: " + relativeDir;
        }

        private void AssetTree_AfterSelect(object sender, TreeViewEventArgs e)
        {
            PopulateItemList();
        }

        private void SelectPath(string path)
        {
            SelectPath_r(path, AssetTree.Nodes[0]);
        }

        private bool SelectPath_r(string path, TreeNode root)
        {
            if (root.Tag.ToString() == path)
            {
                AssetTree.SelectedNode = root;
                return true;
            }

            foreach (TreeNode node in root.Nodes)
            {
                if (SelectPath_r(path, node))
                {
                    return true;
                }
            }

            return false;
        }

        private void OpenAsset(string path)
        {
            AssetType type = AssetHelper.GetAssetType(path);
            switch (type)
            {
            case AssetType.Atlas:
                {
                    break;
                }
            case AssetType.AudioBank:
                {
                    break;
                }
            case AssetType.Font:
                {
                    break;
                }
            case AssetType.Language:
                {
                    break;
                }
            case AssetType.Layout:
                {
                    break;
                }
            case AssetType.Map:
                {
                    break;
                }
            case AssetType.ParticleFX:
                {
                    break;
                }
            case AssetType.Raw:
                {
                    break;
                }
            case AssetType.Script:
                {
                    break;
                }
            case AssetType.Shader:
                {
                    break;
                }
            case AssetType.Video:
                {
                    break;
                }
            case AssetType.UpgradeTreeDB:
                {
                    UpgradeTreeDBEditor form = new UpgradeTreeDBEditor(path);
                    form.Show(this);
                    break;
                }
            }
        }

        private void ItemListView_DoubleClick(object sender, EventArgs e)
        {
            if (ItemListView.SelectedItems.Count == 0)
            {
                return;
            }

            ListViewItem item = ItemListView.SelectedItems[0];
            string path = item.Tag.ToString();
            if (Directory.Exists(path))
            {
                SelectPath(path);
            }
            else
            {
                OpenAsset(path);
            }
        }

        private void UpdateContextMenuState(object sender)
        {
            newToolStripMenuItem.Enabled = (sender == ItemListView);
        }

        private void AssetTree_MouseClick(object sender, MouseEventArgs e)
        {
            UpdateContextMenuState(sender);
        }

        private void ItemListView_MouseClick(object sender, MouseEventArgs e)
        {
            UpdateContextMenuState(sender);
        }

        private void itemToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }
    }
}
