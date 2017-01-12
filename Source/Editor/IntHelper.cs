using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Editor
{
    public static class IntHelper
    {
        private const int PRECISION = 2;

        private static IList<string> Units;

        static IntHelper()
        {
            Units = new List<string>(){
                "B", "KB", "MB", "GB", "TB"
            };
        }

        public static string BytesToHumanReadable(this int bytes)
        {
            double pow = Math.Floor((bytes > 0 ? Math.Log(bytes) : 0) / Math.Log(1024));
            pow = Math.Min(pow, Units.Count - 1);
            double value = (double)bytes / Math.Pow(1024, pow);
            return value.ToString(pow == 0 ? "F0" : "F" + PRECISION.ToString()) + " " + Units[(int)pow];
        }
    }
}
