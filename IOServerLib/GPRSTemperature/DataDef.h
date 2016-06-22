#ifndef _DATADEF_H
#define _DATADEF_H
#include "channel.h"
#include <vector>
#include "GPRSTemperature.h"

//数据到达的回调函数 ColData采集到的所有数据，通过回调函数传入
typedef void(*fnDataArrive)(CHANNEL_INFO *ColData);
//成功解析完一个设备的数据的回调函数，pDevice解析完毕的一个设备数据，通过回调函数传入
//typedef void(*fnGetOneDevice)(CIOCPModel *pParent, CGPRSTemperature *pDevice);

#endif