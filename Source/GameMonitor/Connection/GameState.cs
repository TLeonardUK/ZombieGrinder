using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;

namespace GameMonitor.Connection
{
    public class GameState : BasePacket
    {
        public string Username  = "-";
        public string Map       = "-";
        public string Host      = "-";

        public override void Process(GameConnection Connection)
        {
            Connection.CurrentGameState = this;
            Connection.StateChangeDirty = true;
        }
    }
}
