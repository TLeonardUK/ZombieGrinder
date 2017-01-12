using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public class ScriptProperty
    {
        public string Name;
        public string TypeName;
        public string CurrentValue;
        public bool bReadOnly;

        public List<ScriptProperty> Children = new List<ScriptProperty>();
    }

    public class ResponseScriptProperties : BasePacket
    {
        public bool bScriptExists = false;
        public ScriptProperty Root = new ScriptProperty();

        public override void Process(GameConnection Connection)
        {
        }
    }
}
