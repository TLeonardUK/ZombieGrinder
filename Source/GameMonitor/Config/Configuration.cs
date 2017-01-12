using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Xml.Serialization;
using System.Windows.Forms;

namespace GameMonitor.Config
{
    public class ConnectionConfig
    {
        [XmlAttribute("IP")]
        public string IP;

        [XmlAttribute("Port")]
        public int    Port;
    }

    [Serializable]
    public class ProgramConfiguration
    {
        [XmlArray("Connections"), XmlArrayItem("Connection")]
        public List<ConnectionConfig> Connections = new List<ConnectionConfig>();
    }

    public static class ConfigurationManager
    {
        public static ProgramConfiguration Configuration = new ProgramConfiguration();

        private static string GetSaveLocation()
        {
            return Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData) + @"\GameMonitor\Configuration.xml";
        }

        public static void Save()
        {
            string location = GetSaveLocation();
            string directory = Path.GetDirectoryName(location);

            try
            {
                if (!Directory.Exists(directory))
                {
                    Directory.CreateDirectory(directory);
                }

                XmlSerializer serializer = new XmlSerializer(typeof(ProgramConfiguration));
                TextWriter writer = new StreamWriter(location);
                serializer.Serialize(writer, Configuration);
                writer.Close();
            }
            catch (Exception ex) // Gotta catch em all.
            {
                MessageBox.Show("Failed to save preferences due to exception of type '" + ex.GetType().Name + "'.");
            }
        }

        public static void Load()
        {
            string location = GetSaveLocation();

            if (!File.Exists(location))
            {
                return;
            }

            try
            {
                XmlSerializer serializer = new XmlSerializer(typeof(ProgramConfiguration));
                TextReader reader = new StreamReader(location);
                Configuration = (ProgramConfiguration)serializer.Deserialize(reader);
                reader.Close();
            }
            catch (Exception ex) // Gotta catch em all.
            {
                MessageBox.Show("Failed to load preferences due to exception of type '" + ex.GetType().Name + "'.");
            }
        }
    };
}
