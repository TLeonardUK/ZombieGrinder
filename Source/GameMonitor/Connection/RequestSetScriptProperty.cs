using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public class RequestSetScriptProperty : BasePacket
    {
        public string InstanceName = "";
        public string PropertyPath = "";
        public string PropertyType = "";
        public string NewValue = ""; 

        public override void Process(GameConnection Connection)
        {
        }
    }
}
