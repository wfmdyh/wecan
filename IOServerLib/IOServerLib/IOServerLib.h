#ifndef _IOSERVER_LIB_H
#define _IOSERVER_LIB_H
#include "channel.h"
#include "TransmitManager.h"
//回调函数，参数dv：值的字符串形式的数组
typedef void(__stdcall *PReadColData)(wchar_t **dv,int iCount);
//-----------------------------------函数声明--------------------

/*
 根据通道名，设备名，点名，返回值的指针
 参数：val，返回给调用者
 返回值：找到返回TRUE，没找到返回FALSE
*/
BOOL IOS_GetPointValue(string strChannel,string strDevice,string strPoint,VARIANT *val);

//当通道有数据时的回调函数 pChannels：采集完成的通道数据，多线程
void DataArrive(CHANNEL_INFO *pChannel);
//回调函数 参数：pTraManager，赋值完毕的转发数据
bool UpdataTransmit(TransmitManager *pTraManager);

#endif
