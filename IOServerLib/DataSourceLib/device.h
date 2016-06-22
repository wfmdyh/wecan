#pragma once
#include "point.h"

class DEVICE_INFO
{
public:
	string astrDeviceName;
	vector<COL_POINT*> vecPoint;
	POINT_PARAM mapParam;
	//设备数据采集时间
	string *CTime;
	//运算符重载
	void operator=(DEVICE_INFO &di);
public:
	DEVICE_INFO();
	//释放资源
	~DEVICE_INFO();
};

