#pragma once
#include "point.h"

class DEVICE_INFO
{
public:
	string astrDeviceName;
	vector<COL_POINT*> vecPoint;
	map<string,string> mapParam;
	//�豸���ݲɼ�ʱ��
	string *CTime;
	//���������
	void operator=(DEVICE_INFO &di);
public:
	DEVICE_INFO();
	//�ͷ���Դ
	~DEVICE_INFO();
};

