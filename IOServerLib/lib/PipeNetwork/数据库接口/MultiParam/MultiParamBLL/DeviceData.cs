using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace MultiParamBLL
{
    public class DeviceData
    {
        public int DeviceID { get; set; }        
        public string DeviceCode { get; set; }
        public int MeterStatus { get; set; }
        public int WirelessStatus { get; set; }
        public int PressStatus { get; set; }
        public int Flag { get; set; }
        public double TempValue { get; set; }
        public string TempRecord { get; set; }
        public double PressValue { get; set; }
        public string PressRecord { get; set; }
        public double TotalHeat { get; set; }
        public double TotalFlow { get; set; }
        public double InstantFlow { get; set; }
        public DateTime CollectTime { get; set; }

        public DeviceData()
        {
            MeterStatus = 0;
            WirelessStatus = 0;
            PressStatus = 0;
            Flag = 0;
            TempValue = 0;
            TempRecord = " ";
            PressValue = 0;
            PressRecord = " ";
            TotalHeat = 0;
            TotalFlow = 0;
            InstantFlow = 0;
        }
    }
}
