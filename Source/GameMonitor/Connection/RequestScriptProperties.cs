using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public class RequestScriptProperties : BasePacket
    {
        public string InstanceName = "";

        public override void Process(GameConnection Connection)
        {
        }
    }
}
