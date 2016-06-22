#pragma once
#include "device.h"

class CHANNEL_INFO
{
public:
	string astrName;
	string astrDllFile;
	string astrConfigFileFullPath;
	string astrType;						//链路的传输类型opc com tcp
	vector<DEVICE_INFO*> vecDevice;
	//参数
	map<string, string> mapParam;
	//运算符重载
	void operator=(CHANNEL_INFO &ci);
private:
	wstring m_ID;
private:
	//生成ID
	void GenerateID();
public:
	CHANNEL_INFO();
	//释放资源
	~CHANNEL_INFO();

	wstring GetID();
};

