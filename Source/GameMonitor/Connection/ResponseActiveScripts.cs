using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public class ActiveScriptState
    {
        public string Name;
        public string ClassName;
    }

    public class ResponseActiveScripts : BasePacket
    {
        public List<ActiveScriptState> States = new List<ActiveScriptState>();

        public override void Process(GameConnection Connection)
        {
        }
    }
}
