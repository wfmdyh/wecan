#pragma once

class COL_POINT;

class DEVICE_INFO
{
public:
	string astrDeviceName;
	vector<COL_POINT*> vecPoint;
	map<string, string> mapParam;
	//���������
	void operator=(DEVICE_INFO &di);
public:
	DEVICE_INFO();
	//�ͷ���Դ
	~DEVICE_INFO();
};

