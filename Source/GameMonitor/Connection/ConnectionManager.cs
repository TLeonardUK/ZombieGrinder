using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public static class GameConnectionManager
    {
        public static List<GameConnection> Connections = new List<GameConnection>();

        public static void Poll()
        {
            foreach (GameConnection connection in Connections)
            {
                connection.Poll();
            }
        }
    }
}
