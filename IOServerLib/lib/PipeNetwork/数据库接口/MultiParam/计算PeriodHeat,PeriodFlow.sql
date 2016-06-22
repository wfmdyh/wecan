update DeviceRecord
   set (PeriodHeat, PeriodFlow) =
       (select nvl(DeviceRecord.TotalHeat - R.TotalHeat, 0),
               nvl(DeviceRecord.TotalFlow - R.TotalFlow, 0)
          from DeviceRecord R,
               (select R1.DeviceID,
                       R1.CollectTime,
                       max(R2.CollectTime) PrevCollectTime
                  from DeviceRecord R1, DeviceRecord R2
                 where R1.DeviceID = R2.DeviceID
                   and R1.CollectTime > R2.CollectTime
                 group by R1.DeviceID, R1.CollectTime) RM
         where R.DeviceID = RM.DeviceID
            and DeviceRecord.DeviceID=R.DeviceID
           and R.CollectTime = RM.PrevCollectTime  
           and DeviceRecord.CollectTime = RM.CollectTime);
