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
using GameMonitor.Connection;
using GameMonitor.Utils;

namespace GameMonitor
{
    public partial class ConnectionForm : Form
    {
        public GameConnection Connection = null;
        private string CurrentInstanceName = "";

        public ConnectionForm()
        {
            InitializeComponent();
        }

        private void ConnectionForm_Load(object sender, EventArgs e)
        {
            this.Text = "Connection - " + Connection.Configuration.IP + ":" + Connection.Configuration.Port;
            PopulatePage(0);
        }

        private void TabControl_TabIndexChanged(object sender, EventArgs e)
        {
            PopulatePage(TabControl.SelectedIndex);
        }

        private void AddPropertyChildren(ScriptProperty Property, CustomPropertyGridObject Obj, string path, bool isArray)
        {
            int index = 0;

            foreach (ScriptProperty Child in Property.Children)
            {
                object value = false;
                Type valueType = typeof(bool);

                string nodePath = path;
                if (isArray)
                {
                    nodePath += "[" + index + "]";
                }
                else
                {
                    if (nodePath != "")
                    {
                        nodePath += ".";
                    }
                    nodePath += Child.Name;
                }

                switch (Child.TypeName)
                {
                    case "Bool":
                        {
                            value = (Child.CurrentValue == "1");
                            valueType = typeof(bool);
                            break;
                        }
                    case "String":
                        {
                            value = Child.CurrentValue;
                            valueType = typeof(string);
                            break;
                        }
                    case "Float":
                        {
                            value = float.Parse(Child.CurrentValue);
                            valueType = typeof(float);
                            break;
                        }
                    case "Int":
                        {
                            value = int.Parse(Child.CurrentValue);
                            valueType = typeof(int);
                            break;
                        }
                    case "Array":
                        {
                            if (Child.CurrentValue != "Null")
                            {
                                CustomPropertyGridObject SubChild = new CustomPropertyGridObject();
                                AddPropertyChildren(Child, SubChild, nodePath, true);

                                value = SubChild;
                                valueType = typeof(CustomPropertyGridObject);
                            }
                            else
                            {
                                value = null;
                                valueType = typeof(object);
                            }
                            break;
                        }
                    case "Object":
                        {
                            if (Child.CurrentValue != "Null")
                            {
                                CustomPropertyGridObject SubChild = new CustomPropertyGridObject();
                                AddPropertyChildren(Child, SubChild, nodePath, false);

                                value = SubChild;
                                valueType = typeof(CustomPropertyGridObject);
                            }
                            else
                            {
                                value = null;
                                valueType = typeof(object);
                            }
                            break;
                        }
                }

                string DisplayName = Child.Name.Replace("_", " ");
                if (DisplayName.Length > 2 && DisplayName.Substring(0, 2) == "m ")
                {
                    DisplayName = DisplayName.Substring(2);
                }
                if (DisplayName.Length > 0 && DisplayName[0] >= 'a' && DisplayName[0] <= 'z')
                {
                    DisplayName = DisplayName.Substring(0, 1).ToUpper() + DisplayName.Substring(1);
                }

                bool bReadOnly = Child.bReadOnly;
                if (DisplayName.Length > 3 && Utils.ValidationHelper.IsUppercase(DisplayName))
                {
                    bReadOnly = true;
                }

                if (isArray)
                {
                    DisplayName = "[" + index + "]";
                }

                CustomProperty Prop = new CustomProperty(DisplayName, value, valueType, bReadOnly, true, nodePath);
                Prop.Changed += Property_Changed;
                Obj.Add(Prop);

                index++;
            }
        }

        private void Property_Changed(CustomProperty Prop)
        {
            System.Console.WriteLine(Prop.FullPath + " = " + Prop.Value);

            RequestSetScriptProperty packet = new RequestSetScriptProperty();
            packet.InstanceName = CurrentInstanceName;
            packet.NewValue = Prop.Value.ToString();
            packet.PropertyPath = Prop.FullPath;

            if (Prop.Type == typeof(string))
                packet.PropertyType = "String";
            else if (Prop.Type == typeof(int))
                packet.PropertyType = "Int";
            else if (Prop.Type == typeof(bool))
                packet.PropertyType = "Bool";
            else if (Prop.Type == typeof(float))
                packet.PropertyType = "Float";

            Connection.Send(packet, null);
        }

        private void PopulateProperties(ResponseScriptProperties Packet)
        {
            CustomPropertyGridObject GridObject = new CustomPropertyGridObject();
            AddPropertyChildren(Packet.Root, GridObject, "", false);

            ScriptPropertyGrid.SelectedObject = GridObject;
        }

        private void PopulateActiveScripts(ResponseActiveScripts Packet)
        {
            List<string> UniqueClasses = new List<string>();
            foreach (ActiveScriptState State in Packet.States)
            {
                if (!UniqueClasses.Contains(State.ClassName))
                {
                    UniqueClasses.Add(State.ClassName);
                }
            }

            ActiveScriptsTreeView.Nodes.Clear();

            foreach (string ClassName in UniqueClasses)
            {
                int TotalInstances = 0;
                foreach (ActiveScriptState State in Packet.States)
                {
                    if (State.ClassName == ClassName)
                    {
                        TotalInstances++;
                    }
                }

                string DisplayClassName = "[" + TotalInstances + "] " + ClassName;
                TreeNode ClassNode = ActiveScriptsTreeView.Nodes.Add(ClassName, DisplayClassName, 0, 0);

                foreach (ActiveScriptState State in Packet.States)
                {
                    if (State.ClassName == ClassName)
                    {
                        string InstanceName = "Instance #" + State.Name;
                        TreeNode InstanceNode = ClassNode.Nodes.Add(State.Name, InstanceName, 1, 1);
                    }
                }
            }

            ScriptPropertyGrid.SelectedObject = null;
        }

        private void PopulatePage(int Index)
        {
            switch (TabControl.SelectedIndex)
            {
                // Overview
                case 0:
                    {
                        break;
                    }

                // Active Scripts
                case 1:
                    {
                        RefreshActiveScripts();
                        break;
                    }
            }
        }

        private void ActiveScriptsTreeView_AfterSelect(object sender, TreeViewEventArgs e)
        {
            if (e.Node.Level == 1)
            {
                System.Console.WriteLine("Requesting object properties: "+e.Node.Text);

                RequestScriptProperties packet = new RequestScriptProperties();
                packet.InstanceName = e.Node.Name;

                Connection.Send(packet,
                    new PacketResponseEvent
                    (
                        (GameConnection Reciever, BasePacket Packet) =>
                        {
                            CurrentInstanceName = e.Node.Name;
                            PopulateProperties(Packet as ResponseScriptProperties);
                        }
                    )
                );
            }
        }

        private void RefreshActiveScriptsToolbarItem_Click(object sender, EventArgs e)
        {
            RefreshActiveScripts();
        }

        private void RefreshActiveScripts()
        {
            Connection.Send(new RequestActiveScripts(),
                new PacketResponseEvent
                (
                    (GameConnection Reciever, BasePacket Packet) =>
                    {
                        PopulateActiveScripts(Packet as ResponseActiveScripts);
                    }
                )
            );
        }

        private void pictureBox1_Click(object sender, EventArgs e)
        {

        }
    }
}
