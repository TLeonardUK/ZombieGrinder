using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GameMonitor.Utils
{
    public static class ValidationHelper
    {
        public static bool IsValidIP(string IP)
        {
            string[] segments = IP.Split('.');
            if (segments.Length == 4)
            {
                for (int i = 0; i < 4; i++)
                {
                    int Result = 0;

                    if (segments[i].Length > 0 && int.TryParse(segments[i], out Result))
                    {
                        if (Result < 0 || Result > 255)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        return false;
                    }
                }

                return true;
            }
            return false;
        }

        public static bool IsValidPort(string Port)
        {
            int Result = 0;
            if (int.TryParse(Port, out Result))
            {
                if (Result > 0 && Result < 65356)
                {
                    return true;
                }
            }
            return false;
        }

        public static bool IsUppercase(string str)
        {
            for (int i = 0; i < str.Length; i++)
            {
                if (str[i] >= 'a' && str[i] <= 'z')
                {
                    return false;
                }
            }
            return true;
        }
    }
}
