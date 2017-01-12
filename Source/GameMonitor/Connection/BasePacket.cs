using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Connection
{
    public class BasePacket
    {
        private static int IDCounter = 0;

        public string ClassName = "";
        public int ID = 0;
        public int RequestID = 0;

        public BasePacket()
        {
            ID = ++IDCounter;
            ClassName = GetType().Name;
        }

        public virtual void Process(GameConnection Connection)
        {
            throw new NotImplementedException();
        }
    }
}
