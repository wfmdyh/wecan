#pragma once
#include "device.h"

class CHANNEL_INFO
{
public:
	string astrName;
	string astrDllFile;
	string astrConfigFileFullPath;
	string astrType;	//��·�Ĵ�������opc com tcp
	vector<DEVICE_INFO*> vecDevice;
	//����
	POINT_PARAM mapParam;
	//���������
	void operator=(CHANNEL_INFO &ci);
public:
	CHANNEL_INFO();
	//�ͷ���Դ
	~CHANNEL_INFO();
};
