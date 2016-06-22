#pragma once
//#include "device.h"

class DEVICE_INFO;

class CHANNEL_INFO
{
public:
	string astrName;
	string astrDllFile;
	string astrConfigFileFullPath;
	string astrType;	//链路的传输类型opc com tcp
	vector<DEVICE_INFO*> vecDevice;
	//参数
	map<string,string> mapParam;
	//运算符重载
	void operator=(CHANNEL_INFO &ci);
public:
	CHANNEL_INFO();
	//释放资源
	~CHANNEL_INFO();
};

