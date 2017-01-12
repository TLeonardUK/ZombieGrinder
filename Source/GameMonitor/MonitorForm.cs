using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using GameMonitor.Config;
using GameMonitor.Utils;
using GameMonitor.Connection;

namespace GameMonitor
{
    public partial class MonitorForm : Form
    {
        public List<ConnectionForm> ConnectionForms = new List<ConnectionForm>();

        public MonitorForm()
        {
            InitializeComponent();
        }

        private void MonitorForm_Load(object sender, EventArgs e)
        {
            ConfigurationManager.Load();

            foreach (ConnectionConfig Config in ConfigurationManager.Configuration.Connections)
            {
                GameConnectionManager.Connections.Add(new GameConnection(Config));
            }

            ReloadConnectionList();
        }

        private void AddConnectionButton_Click(object sender, EventArgs e)
        {
            ConnectionConfig config = new ConnectionConfig();
            config.IP = IpAddressTextBox.Text;
            config.Port = int.Parse(PortTextBox.Text);

            ConfigurationManager.Configuration.Connections.Add(config);
            ConfigurationManager.Save();

            GameConnectionManager.Connections.Add(new GameConnection(config));

            ReloadConnectionList();
        }

        private void ReloadConnectionList()
        {
            ConnectionListView.Items.Clear();

            foreach (ConnectionConfig Config in ConfigurationManager.Configuration.Connections)
            {
                ListViewItem Item = new ListViewItem();
                Item.Text = Config.IP + ":" + Config.Port;
                Item.SubItems.Add("Disconnected");
                Item.SubItems.Add("-");
                Item.SubItems.Add("-");
                Item.SubItems.Add("-");
                ConnectionListView.Items.Add(Item);
            }
        }

        private void RefreshConnectionList()
        {
            for (int i = 0; i < GameConnectionManager.Connections.Count; i++)
            {
                GameConnection Connection = GameConnectionManager.Connections[i];
                ListViewItem Item = ConnectionListView.Items[i];

                if (!Connection.HasStateChanged())
                {
                    break;
                }

                Item.Text = Connection.Configuration.IP + ":" + Connection.Configuration.Port;

                switch (Connection.State)
                {
                    case GameConnectionState.Connected:
                        {
                            Item.SubItems[1].Text = "Connected";
                            Item.SubItems[2].Text = Connection.CurrentGameState.Username;
                            Item.SubItems[3].Text = Connection.CurrentGameState.Map;
                            Item.SubItems[4].Text = Connection.CurrentGameState.Host;
                            break;
                        }
                    case GameConnectionState.Connecting:
                        {
                            Item.SubItems[1].Text = "Connecting ...";
                            Item.SubItems[2].Text = "-";
                            Item.SubItems[3].Text = "-";
                            Item.SubItems[4].Text = "-";
                            break;
                        }
                    case GameConnectionState.Disconnected:
                        {
                            Item.SubItems[1].Text = "Disconnected";
                            Item.SubItems[2].Text = "-";
                            Item.SubItems[3].Text = "-";
                            Item.SubItems[4].Text = "-";

                            // Destroy form if we have disconnected.
                            foreach (ConnectionForm OldForm in ConnectionForms)
                            {
                                if (Connection == OldForm.Connection)
                                {
                                    OldForm.Close();
                                    ConnectionForms.Remove(OldForm);
                                    break;
                                }
                            }

                            break;
                        }
                }
            }
        }

        private void AddressTextBox_TextChanged(object sender, EventArgs e)
        {
            bool ValidInfo = (ValidationHelper.IsValidIP(IpAddressTextBox.Text) && ValidationHelper.IsValidPort(PortTextBox.Text));
            bool IsDuplicate = false;

            foreach (ConnectionConfig Config in ConfigurationManager.Configuration.Connections)
            {
                if (Config.IP == IpAddressTextBox.Text && Config.Port.ToString() == PortTextBox.Text)
                {
                    IsDuplicate = true;
                    break;
                }
            }

            AddConnectionButton.Enabled = ValidInfo && !IsDuplicate;
        }

        private void ConnectionListView_SelectedIndexChanged(object sender, EventArgs e)
        {
            RemoveConnectionButton.Enabled = (ConnectionListView.SelectedItems.Count > 0);
        }

        private void RemoveConnectionButton_Click(object sender, EventArgs e)
        {
            List<ConnectionConfig> RemovedStates = new List<ConnectionConfig>();
            List<GameConnection> RemovedConnections = new List<GameConnection>();

            foreach (int index in ConnectionListView.SelectedIndices)
            {
                ConnectionConfig State = ConfigurationManager.Configuration.Connections[index];
                GameConnection RemoveConnection = GameConnectionManager.Connections[index];
                RemovedStates.Add(State);
                RemovedConnections.Add(RemoveConnection);
            }

            List<ConnectionForm> FormsCopy = new List<ConnectionForm>(ConnectionForms);
            foreach (ConnectionForm OldForm in FormsCopy)
            {
                if (RemovedConnections.Contains(OldForm.Connection))
                {
                    OldForm.Close();
                    ConnectionForms.Remove(OldForm);
                }
            }

            foreach (ConnectionConfig config in RemovedStates)
            {
                ConfigurationManager.Configuration.Connections.Remove(config);
            }

            foreach (GameConnection connection in RemovedConnections)
            {
                GameConnectionManager.Connections.Remove(connection);
            }

            ConfigurationManager.Save();

            ReloadConnectionList();
        }

        private void ConnectionPollTimer_Tick(object sender, EventArgs e)
        {
            GameConnectionManager.Poll();

            List<ConnectionForm> FormsCopy = new List<ConnectionForm>(ConnectionForms);
            foreach (ConnectionForm OldForm in FormsCopy)
            {
                if (OldForm.IsDisposed)
                {
                    ConnectionForms.Remove(OldForm);
                }
            }


            RefreshConnectionList();
        }

        private void ConnectionListView_ItemActivate(object sender, EventArgs e)
        {
            foreach (int index in ConnectionListView.SelectedIndices)
            {
                GameConnection Connection = GameConnectionManager.Connections[index];
                if (Connection.State == GameConnectionState.Connected)
                {
                    bool bIsNew = true;

                    foreach (ConnectionForm OldForm in ConnectionForms)
                    {
                        if (OldForm.Connection == Connection)
                        {
                            OldForm.BringToFront();
                            OldForm.Activate();
                            bIsNew = false;
                            break;
                        }
                    }

                    if (bIsNew)
                    {
                        ConnectionForm NewForm = new ConnectionForm();
                        NewForm.Connection = Connection;
                        NewForm.Show(this);

                        ConnectionForms.Add(NewForm);
                    }
                }
            }
        }
    }
}
