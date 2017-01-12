namespace Editor
{
    partial class MainForm
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
        if (disposing && (components != null))
        {
        components.Dispose();
        }
        base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MainForm));
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.AssetTree = new System.Windows.Forms.TreeView();
            this.folderListContentMenu = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.atlasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.audioBankToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.fontToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.languageToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.layoutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mapToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.particleFXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.rawToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scriptToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.emptyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.itemToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.enemyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.weaponToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.achievementToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.entityToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pickupToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.challengeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.dropTableToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.postEffectToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.gameModeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.skillToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.statisticToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tutorialToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.shaderToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.videoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
            this.editToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.deleteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.renameToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.duplicateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator4 = new System.Windows.Forms.ToolStripSeparator();
            this.cutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.copyToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.pasteToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.folderImageList = new System.Windows.Forms.ImageList(this.components);
            this.ItemListView = new System.Windows.Forms.ListView();
            this.assetTypeColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.fileSizeColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.fileNameColumn = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.StatusStrip = new System.Windows.Forms.StatusStrip();
            this.ModNameLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.AssetPathLabel = new System.Windows.Forms.ToolStripStatusLabel();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.OpenProjectButton = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.ExitButton = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.folderListContentMenu.SuspendLayout();
            this.StatusStrip.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.Location = new System.Drawing.Point(0, 24);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.AssetTree);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.ItemListView);
            this.splitContainer1.Size = new System.Drawing.Size(721, 372);
            this.splitContainer1.SplitterDistance = 239;
            this.splitContainer1.TabIndex = 0;
            // 
            // AssetTree
            // 
            this.AssetTree.ContextMenuStrip = this.folderListContentMenu;
            this.AssetTree.Dock = System.Windows.Forms.DockStyle.Fill;
            this.AssetTree.FullRowSelect = true;
            this.AssetTree.ImageIndex = 0;
            this.AssetTree.ImageList = this.folderImageList;
            this.AssetTree.Location = new System.Drawing.Point(0, 0);
            this.AssetTree.Name = "AssetTree";
            this.AssetTree.SelectedImageIndex = 0;
            this.AssetTree.Size = new System.Drawing.Size(239, 372);
            this.AssetTree.TabIndex = 0;
            this.AssetTree.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.AssetTree_AfterSelect);
            this.AssetTree.MouseClick += new System.Windows.Forms.MouseEventHandler(this.AssetTree_MouseClick);
            // 
            // folderListContentMenu
            // 
            this.folderListContentMenu.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripMenuItem1,
            this.newToolStripMenuItem,
            this.toolStripSeparator3,
            this.editToolStripMenuItem,
            this.deleteToolStripMenuItem,
            this.renameToolStripMenuItem,
            this.duplicateToolStripMenuItem,
            this.toolStripSeparator4,
            this.cutToolStripMenuItem,
            this.copyToolStripMenuItem,
            this.pasteToolStripMenuItem});
            this.folderListContentMenu.Name = "folderListContentMenu";
            this.folderListContentMenu.Size = new System.Drawing.Size(153, 236);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Enabled = false;
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(152, 22);
            this.toolStripMenuItem1.Text = "New Folder";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.atlasToolStripMenuItem,
            this.audioBankToolStripMenuItem,
            this.fontToolStripMenuItem,
            this.languageToolStripMenuItem,
            this.layoutToolStripMenuItem,
            this.mapToolStripMenuItem,
            this.particleFXToolStripMenuItem,
            this.rawToolStripMenuItem,
            this.scriptToolStripMenuItem,
            this.shaderToolStripMenuItem,
            this.videoToolStripMenuItem});
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.newToolStripMenuItem.Text = "New File";
            // 
            // atlasToolStripMenuItem
            // 
            this.atlasToolStripMenuItem.Enabled = false;
            this.atlasToolStripMenuItem.Name = "atlasToolStripMenuItem";
            this.atlasToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.atlasToolStripMenuItem.Text = "Atlas";
            // 
            // audioBankToolStripMenuItem
            // 
            this.audioBankToolStripMenuItem.Enabled = false;
            this.audioBankToolStripMenuItem.Name = "audioBankToolStripMenuItem";
            this.audioBankToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.audioBankToolStripMenuItem.Text = "Audio Bank";
            // 
            // fontToolStripMenuItem
            // 
            this.fontToolStripMenuItem.Enabled = false;
            this.fontToolStripMenuItem.Name = "fontToolStripMenuItem";
            this.fontToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.fontToolStripMenuItem.Text = "Font";
            // 
            // languageToolStripMenuItem
            // 
            this.languageToolStripMenuItem.Enabled = false;
            this.languageToolStripMenuItem.Name = "languageToolStripMenuItem";
            this.languageToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.languageToolStripMenuItem.Text = "Language";
            // 
            // layoutToolStripMenuItem
            // 
            this.layoutToolStripMenuItem.Enabled = false;
            this.layoutToolStripMenuItem.Name = "layoutToolStripMenuItem";
            this.layoutToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.layoutToolStripMenuItem.Text = "Layout";
            // 
            // mapToolStripMenuItem
            // 
            this.mapToolStripMenuItem.Enabled = false;
            this.mapToolStripMenuItem.Name = "mapToolStripMenuItem";
            this.mapToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.mapToolStripMenuItem.Text = "Map";
            // 
            // particleFXToolStripMenuItem
            // 
            this.particleFXToolStripMenuItem.Enabled = false;
            this.particleFXToolStripMenuItem.Name = "particleFXToolStripMenuItem";
            this.particleFXToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.particleFXToolStripMenuItem.Text = "Particle FX";
            // 
            // rawToolStripMenuItem
            // 
            this.rawToolStripMenuItem.Enabled = false;
            this.rawToolStripMenuItem.Name = "rawToolStripMenuItem";
            this.rawToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.rawToolStripMenuItem.Text = "Raw";
            // 
            // scriptToolStripMenuItem
            // 
            this.scriptToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.emptyToolStripMenuItem,
            this.toolStripSeparator2,
            this.itemToolStripMenuItem,
            this.enemyToolStripMenuItem,
            this.weaponToolStripMenuItem,
            this.achievementToolStripMenuItem,
            this.entityToolStripMenuItem,
            this.pickupToolStripMenuItem,
            this.challengeToolStripMenuItem,
            this.dropTableToolStripMenuItem,
            this.postEffectToolStripMenuItem,
            this.gameModeToolStripMenuItem,
            this.skillToolStripMenuItem,
            this.statisticToolStripMenuItem,
            this.tutorialToolStripMenuItem});
            this.scriptToolStripMenuItem.Name = "scriptToolStripMenuItem";
            this.scriptToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.scriptToolStripMenuItem.Text = "Script";
            // 
            // emptyToolStripMenuItem
            // 
            this.emptyToolStripMenuItem.Enabled = false;
            this.emptyToolStripMenuItem.Name = "emptyToolStripMenuItem";
            this.emptyToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.emptyToolStripMenuItem.Text = "Empty";
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(141, 6);
            // 
            // itemToolStripMenuItem
            // 
            this.itemToolStripMenuItem.Name = "itemToolStripMenuItem";
            this.itemToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.itemToolStripMenuItem.Text = "Item";
            this.itemToolStripMenuItem.Click += new System.EventHandler(this.itemToolStripMenuItem_Click);
            // 
            // enemyToolStripMenuItem
            // 
            this.enemyToolStripMenuItem.Enabled = false;
            this.enemyToolStripMenuItem.Name = "enemyToolStripMenuItem";
            this.enemyToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.enemyToolStripMenuItem.Text = "Enemy";
            // 
            // weaponToolStripMenuItem
            // 
            this.weaponToolStripMenuItem.Enabled = false;
            this.weaponToolStripMenuItem.Name = "weaponToolStripMenuItem";
            this.weaponToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.weaponToolStripMenuItem.Text = "Weapon";
            // 
            // achievementToolStripMenuItem
            // 
            this.achievementToolStripMenuItem.Enabled = false;
            this.achievementToolStripMenuItem.Name = "achievementToolStripMenuItem";
            this.achievementToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.achievementToolStripMenuItem.Text = "Achievement";
            // 
            // entityToolStripMenuItem
            // 
            this.entityToolStripMenuItem.Enabled = false;
            this.entityToolStripMenuItem.Name = "entityToolStripMenuItem";
            this.entityToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.entityToolStripMenuItem.Text = "Entity";
            // 
            // pickupToolStripMenuItem
            // 
            this.pickupToolStripMenuItem.Enabled = false;
            this.pickupToolStripMenuItem.Name = "pickupToolStripMenuItem";
            this.pickupToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.pickupToolStripMenuItem.Text = "Pickup";
            // 
            // challengeToolStripMenuItem
            // 
            this.challengeToolStripMenuItem.Enabled = false;
            this.challengeToolStripMenuItem.Name = "challengeToolStripMenuItem";
            this.challengeToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.challengeToolStripMenuItem.Text = "Challenge";
            // 
            // dropTableToolStripMenuItem
            // 
            this.dropTableToolStripMenuItem.Enabled = false;
            this.dropTableToolStripMenuItem.Name = "dropTableToolStripMenuItem";
            this.dropTableToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.dropTableToolStripMenuItem.Text = "Drop Table";
            // 
            // postEffectToolStripMenuItem
            // 
            this.postEffectToolStripMenuItem.Enabled = false;
            this.postEffectToolStripMenuItem.Name = "postEffectToolStripMenuItem";
            this.postEffectToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.postEffectToolStripMenuItem.Text = "Post Effect";
            // 
            // gameModeToolStripMenuItem
            // 
            this.gameModeToolStripMenuItem.Enabled = false;
            this.gameModeToolStripMenuItem.Name = "gameModeToolStripMenuItem";
            this.gameModeToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.gameModeToolStripMenuItem.Text = "Game Mode";
            // 
            // skillToolStripMenuItem
            // 
            this.skillToolStripMenuItem.Enabled = false;
            this.skillToolStripMenuItem.Name = "skillToolStripMenuItem";
            this.skillToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.skillToolStripMenuItem.Text = "Skill";
            // 
            // statisticToolStripMenuItem
            // 
            this.statisticToolStripMenuItem.Enabled = false;
            this.statisticToolStripMenuItem.Name = "statisticToolStripMenuItem";
            this.statisticToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.statisticToolStripMenuItem.Text = "Statistic";
            // 
            // tutorialToolStripMenuItem
            // 
            this.tutorialToolStripMenuItem.Enabled = false;
            this.tutorialToolStripMenuItem.Name = "tutorialToolStripMenuItem";
            this.tutorialToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.tutorialToolStripMenuItem.Text = "Tutorial";
            // 
            // shaderToolStripMenuItem
            // 
            this.shaderToolStripMenuItem.Enabled = false;
            this.shaderToolStripMenuItem.Name = "shaderToolStripMenuItem";
            this.shaderToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.shaderToolStripMenuItem.Text = "Shader";
            // 
            // videoToolStripMenuItem
            // 
            this.videoToolStripMenuItem.Enabled = false;
            this.videoToolStripMenuItem.Name = "videoToolStripMenuItem";
            this.videoToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.videoToolStripMenuItem.Text = "Video";
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(149, 6);
            // 
            // editToolStripMenuItem
            // 
            this.editToolStripMenuItem.Enabled = false;
            this.editToolStripMenuItem.Name = "editToolStripMenuItem";
            this.editToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.editToolStripMenuItem.Text = "Edit";
            // 
            // deleteToolStripMenuItem
            // 
            this.deleteToolStripMenuItem.Enabled = false;
            this.deleteToolStripMenuItem.Name = "deleteToolStripMenuItem";
            this.deleteToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.deleteToolStripMenuItem.Text = "Delete";
            // 
            // renameToolStripMenuItem
            // 
            this.renameToolStripMenuItem.Enabled = false;
            this.renameToolStripMenuItem.Name = "renameToolStripMenuItem";
            this.renameToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.renameToolStripMenuItem.Text = "Rename";
            // 
            // duplicateToolStripMenuItem
            // 
            this.duplicateToolStripMenuItem.Enabled = false;
            this.duplicateToolStripMenuItem.Name = "duplicateToolStripMenuItem";
            this.duplicateToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.duplicateToolStripMenuItem.Text = "Duplicate";
            // 
            // toolStripSeparator4
            // 
            this.toolStripSeparator4.Name = "toolStripSeparator4";
            this.toolStripSeparator4.Size = new System.Drawing.Size(149, 6);
            // 
            // cutToolStripMenuItem
            // 
            this.cutToolStripMenuItem.Enabled = false;
            this.cutToolStripMenuItem.Name = "cutToolStripMenuItem";
            this.cutToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.cutToolStripMenuItem.Text = "Cut";
            // 
            // copyToolStripMenuItem
            // 
            this.copyToolStripMenuItem.Enabled = false;
            this.copyToolStripMenuItem.Name = "copyToolStripMenuItem";
            this.copyToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.copyToolStripMenuItem.Text = "Copy";
            // 
            // pasteToolStripMenuItem
            // 
            this.pasteToolStripMenuItem.Enabled = false;
            this.pasteToolStripMenuItem.Name = "pasteToolStripMenuItem";
            this.pasteToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.pasteToolStripMenuItem.Text = "Paste";
            // 
            // folderImageList
            // 
            this.folderImageList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("folderImageList.ImageStream")));
            this.folderImageList.TransparentColor = System.Drawing.Color.Transparent;
            this.folderImageList.Images.SetKeyName(0, "folder.png");
            this.folderImageList.Images.SetKeyName(1, "application.png");
            this.folderImageList.Images.SetKeyName(2, "page_white.png");
            // 
            // ItemListView
            // 
            this.ItemListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.assetTypeColumn,
            this.fileSizeColumn,
            this.fileNameColumn});
            this.ItemListView.ContextMenuStrip = this.folderListContentMenu;
            this.ItemListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ItemListView.FullRowSelect = true;
            this.ItemListView.LargeImageList = this.folderImageList;
            this.ItemListView.Location = new System.Drawing.Point(0, 0);
            this.ItemListView.MultiSelect = false;
            this.ItemListView.Name = "ItemListView";
            this.ItemListView.Size = new System.Drawing.Size(478, 372);
            this.ItemListView.SmallImageList = this.folderImageList;
            this.ItemListView.TabIndex = 0;
            this.ItemListView.UseCompatibleStateImageBehavior = false;
            this.ItemListView.View = System.Windows.Forms.View.Details;
            this.ItemListView.DoubleClick += new System.EventHandler(this.ItemListView_DoubleClick);
            this.ItemListView.MouseClick += new System.Windows.Forms.MouseEventHandler(this.ItemListView_MouseClick);
            // 
            // assetTypeColumn
            // 
            this.assetTypeColumn.Text = "Type";
            this.assetTypeColumn.Width = 74;
            // 
            // fileSizeColumn
            // 
            this.fileSizeColumn.Text = "Size";
            this.fileSizeColumn.TextAlign = System.Windows.Forms.HorizontalAlignment.Right;
            this.fileSizeColumn.Width = 73;
            // 
            // fileNameColumn
            // 
            this.fileNameColumn.Text = "Name";
            this.fileNameColumn.Width = 320;
            // 
            // StatusStrip
            // 
            this.StatusStrip.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.ModNameLabel,
            this.AssetPathLabel});
            this.StatusStrip.Location = new System.Drawing.Point(0, 396);
            this.StatusStrip.Name = "StatusStrip";
            this.StatusStrip.Size = new System.Drawing.Size(721, 22);
            this.StatusStrip.TabIndex = 1;
            this.StatusStrip.Text = "statusStrip1";
            // 
            // ModNameLabel
            // 
            this.ModNameLabel.Margin = new System.Windows.Forms.Padding(0, 3, 10, 2);
            this.ModNameLabel.Name = "ModNameLabel";
            this.ModNameLabel.Size = new System.Drawing.Size(38, 17);
            this.ModNameLabel.Text = "Mod: ";
            // 
            // AssetPathLabel
            // 
            this.AssetPathLabel.Name = "AssetPathLabel";
            this.AssetPathLabel.Size = new System.Drawing.Size(37, 17);
            this.AssetPathLabel.Text = "Path: ";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(721, 24);
            this.menuStrip1.TabIndex = 2;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.OpenProjectButton,
            this.toolStripSeparator1,
            this.ExitButton});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // OpenProjectButton
            // 
            this.OpenProjectButton.Name = "OpenProjectButton";
            this.OpenProjectButton.Size = new System.Drawing.Size(143, 22);
            this.OpenProjectButton.Text = "Open Mod ...";
            this.OpenProjectButton.Click += new System.EventHandler(this.OpenProjectButton_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(140, 6);
            // 
            // ExitButton
            // 
            this.ExitButton.Name = "ExitButton";
            this.ExitButton.Size = new System.Drawing.Size(143, 22);
            this.ExitButton.Text = "Exit";
            this.ExitButton.Click += new System.EventHandler(this.ExitButton_Click);
            // 
            // MainForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(721, 418);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.StatusStrip);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MainForm";
            this.ShowIcon = false;
            this.Text = "Zombie Grinder - Content Generator";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.folderListContentMenu.ResumeLayout(false);
            this.StatusStrip.ResumeLayout(false);
            this.StatusStrip.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.TreeView AssetTree;
        private System.Windows.Forms.ListView ItemListView;
        private System.Windows.Forms.StatusStrip StatusStrip;
        private System.Windows.Forms.ToolStripStatusLabel AssetPathLabel;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem OpenProjectButton;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.ToolStripMenuItem ExitButton;
        private System.Windows.Forms.ToolStripStatusLabel ModNameLabel;
        private System.Windows.Forms.ImageList folderImageList;
        private System.Windows.Forms.ColumnHeader assetTypeColumn;
        private System.Windows.Forms.ColumnHeader fileSizeColumn;
        private System.Windows.Forms.ColumnHeader fileNameColumn;
        private System.Windows.Forms.ContextMenuStrip folderListContentMenu;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem atlasToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem audioBankToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fontToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem languageToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem layoutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mapToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem particleFXToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem rawToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem scriptToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem shaderToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem videoToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem itemToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem enemyToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem weaponToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem achievementToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem entityToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pickupToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem challengeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem emptyToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
        private System.Windows.Forms.ToolStripMenuItem dropTableToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem postEffectToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem gameModeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem skillToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem statisticToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem tutorialToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator3;
        private System.Windows.Forms.ToolStripMenuItem deleteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem renameToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem duplicateToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator4;
        private System.Windows.Forms.ToolStripMenuItem cutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem copyToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem pasteToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem editToolStripMenuItem;
    }
}

