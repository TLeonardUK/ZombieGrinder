namespace GameMonitor
{
    partial class MonitorForm
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
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.toolStripLabel1 = new System.Windows.Forms.ToolStripLabel();
            this.IpAddressTextBox = new System.Windows.Forms.ToolStripTextBox();
            this.toolStripLabel2 = new System.Windows.Forms.ToolStripLabel();
            this.PortTextBox = new System.Windows.Forms.ToolStripTextBox();
            this.AddConnectionButton = new System.Windows.Forms.ToolStripButton();
            this.RemoveConnectionButton = new System.Windows.Forms.ToolStripButton();
            this.ConnectionListView = new System.Windows.Forms.ListView();
            this.Address = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.State = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Username = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Map = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.Host = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.ConnectionPollTimer = new System.Windows.Forms.Timer(this.components);
            this.toolStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripLabel1,
            this.IpAddressTextBox,
            this.toolStripLabel2,
            this.PortTextBox,
            this.AddConnectionButton,
            this.RemoveConnectionButton});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(684, 25);
            this.toolStrip1.TabIndex = 0;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // toolStripLabel1
            // 
            this.toolStripLabel1.Name = "toolStripLabel1";
            this.toolStripLabel1.Size = new System.Drawing.Size(65, 22);
            this.toolStripLabel1.Text = "IP Address:";
            // 
            // IpAddressTextBox
            // 
            this.IpAddressTextBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.IpAddressTextBox.Name = "IpAddressTextBox";
            this.IpAddressTextBox.Size = new System.Drawing.Size(100, 25);
            this.IpAddressTextBox.TextChanged += new System.EventHandler(this.AddressTextBox_TextChanged);
            // 
            // toolStripLabel2
            // 
            this.toolStripLabel2.Name = "toolStripLabel2";
            this.toolStripLabel2.Size = new System.Drawing.Size(32, 22);
            this.toolStripLabel2.Text = "Port:";
            // 
            // PortTextBox
            // 
            this.PortTextBox.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.PortTextBox.Name = "PortTextBox";
            this.PortTextBox.Size = new System.Drawing.Size(100, 25);
            this.PortTextBox.Text = "34567";
            this.PortTextBox.TextChanged += new System.EventHandler(this.AddressTextBox_TextChanged);
            // 
            // AddConnectionButton
            // 
            this.AddConnectionButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.AddConnectionButton.Enabled = false;
            this.AddConnectionButton.Image = global::GameMonitor.Properties.Resources.add;
            this.AddConnectionButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.AddConnectionButton.Name = "AddConnectionButton";
            this.AddConnectionButton.Size = new System.Drawing.Size(23, 22);
            this.AddConnectionButton.Text = "Add Connection";
            this.AddConnectionButton.Click += new System.EventHandler(this.AddConnectionButton_Click);
            // 
            // RemoveConnectionButton
            // 
            this.RemoveConnectionButton.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.RemoveConnectionButton.Enabled = false;
            this.RemoveConnectionButton.Image = global::GameMonitor.Properties.Resources.delete;
            this.RemoveConnectionButton.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.RemoveConnectionButton.Name = "RemoveConnectionButton";
            this.RemoveConnectionButton.Size = new System.Drawing.Size(23, 22);
            this.RemoveConnectionButton.Text = "Remove Connection";
            this.RemoveConnectionButton.Click += new System.EventHandler(this.RemoveConnectionButton_Click);
            // 
            // ConnectionListView
            // 
            this.ConnectionListView.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.Address,
            this.State,
            this.Username,
            this.Map,
            this.Host});
            this.ConnectionListView.Dock = System.Windows.Forms.DockStyle.Fill;
            this.ConnectionListView.FullRowSelect = true;
            this.ConnectionListView.GridLines = true;
            this.ConnectionListView.Location = new System.Drawing.Point(0, 25);
            this.ConnectionListView.Name = "ConnectionListView";
            this.ConnectionListView.Size = new System.Drawing.Size(684, 182);
            this.ConnectionListView.TabIndex = 1;
            this.ConnectionListView.UseCompatibleStateImageBehavior = false;
            this.ConnectionListView.View = System.Windows.Forms.View.Details;
            this.ConnectionListView.ItemActivate += new System.EventHandler(this.ConnectionListView_ItemActivate);
            this.ConnectionListView.SelectedIndexChanged += new System.EventHandler(this.ConnectionListView_SelectedIndexChanged);
            // 
            // Address
            // 
            this.Address.Text = "Address";
            this.Address.Width = 115;
            // 
            // State
            // 
            this.State.Text = "State";
            this.State.Width = 103;
            // 
            // Username
            // 
            this.Username.Text = "Username";
            this.Username.Width = 176;
            // 
            // Map
            // 
            this.Map.Text = "Game Map";
            this.Map.Width = 141;
            // 
            // Host
            // 
            this.Host.Text = "Game Host";
            this.Host.Width = 137;
            // 
            // ConnectionPollTimer
            // 
            this.ConnectionPollTimer.Enabled = true;
            this.ConnectionPollTimer.Interval = 16;
            this.ConnectionPollTimer.Tick += new System.EventHandler(this.ConnectionPollTimer_Tick);
            // 
            // MonitorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(684, 207);
            this.Controls.Add(this.ConnectionListView);
            this.Controls.Add(this.toolStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "MonitorForm";
            this.ShowIcon = false;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "Game Monitor";
            this.Load += new System.EventHandler(this.MonitorForm_Load);
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripLabel toolStripLabel1;
        private System.Windows.Forms.ToolStripTextBox IpAddressTextBox;
        private System.Windows.Forms.ToolStripLabel toolStripLabel2;
        private System.Windows.Forms.ToolStripTextBox PortTextBox;
        private System.Windows.Forms.ToolStripButton AddConnectionButton;
        private System.Windows.Forms.ListView ConnectionListView;
        private System.Windows.Forms.ColumnHeader Address;
        private System.Windows.Forms.ColumnHeader State;
        private System.Windows.Forms.ColumnHeader Map;
        private System.Windows.Forms.ColumnHeader Host;
        private System.Windows.Forms.ToolStripButton RemoveConnectionButton;
        private System.Windows.Forms.Timer ConnectionPollTimer;
        private System.Windows.Forms.ColumnHeader Username;

    }
}

