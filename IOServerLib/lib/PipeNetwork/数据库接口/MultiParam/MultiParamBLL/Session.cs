using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using DatabaseServer;

namespace MultiParamBLL
{
    public class Session
    {
        public string Error { get; set; }        
        private IDatabaseServer db;
                
        public Session() { db = DBFactory.CreateDB(); }

        public bool Open()
        {
            string sql = "";
            try { db.Open(); }
            catch (Exception ex)
            {
                Error = ex.Message + ";" + sql;
                return false;
            }
            return true;
        }

        public bool Close()
        {
            try { db.Close(); }
            catch (Exception ex)
            {
                Error = ex.Message;
                return false;
            }
            return true;
        }

        /// <summary>
        /// 保存数据
        /// </summary>
        /// <param name="data"></param>
        /// <returns></returns>
        public bool SaveDeviceData(DeviceData data)
        {
            string sql = "";

            try
            {
                data.DeviceID = GetDeviceID(data.DeviceCode);

                sql = "update DeviceData set " +
                    "MeterStatus=" + data.MeterStatus + "," +
                    "WirelessStatus=" + data.WirelessStatus + "," +
                    "PressStatus=" + data.PressStatus + "," +
                    "Flag=" + data.Flag + "," +
                    "TempValue=" + data.TempValue + "," +
                    "TempRecord='" + data.TempRecord + "'," +
                    "PressValue=" + data.PressValue + "," +
                    "PressRecord='" + data.PressRecord + "'," +
                    "TotalHeat=" + data.TotalHeat + "," +
                    "TotalFlow=" + data.TotalFlow + "," +
                    "InstantFlow=" + data.InstantFlow + "," +
                    "CollectTime=sysdate where DeviceID=" + data.DeviceID;
                db.ExecuteVoidSql(sql);

                sql = "SaveDeviceRecord";
                db.ExecuteProcedure(sql, "device_id", data.DeviceID);
            }
            catch (Exception ex)
            {
                Error = ex.Message + "；" + sql;
                return false;
            }

            return true;
        }

        /// <summary>
        /// 取设备ID
        /// </summary>
        /// <param name="device_code"></param>
        /// <returns></returns>
        private int GetDeviceID(string device_code)
        {
            int device_id;
            string sql;

            sql = "select DeviceID from Device where DeviceCode='" + device_code + "'";
            device_id = db.ExecuteIntSql(sql);
            return device_id;
        }
    }
}
