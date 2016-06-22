#ifndef _DATADEF_H
#define _DATADEF_H
#include "channel.h"

//数据到达的回调函数 ColData采集到的所有数据，通过回调函数传入
typedef void(*fnDataArrive)(CHANNEL_INFO *ColData);


#endif