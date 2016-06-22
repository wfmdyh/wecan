#pragma once
#include "device.h"

class CHANNEL_INFO
{
public:
	string astrName;
	string astrDllFile;
	string astrConfigFileFullPath;
	string astrType;						//��·�Ĵ�������opc com tcp
	vector<DEVICE_INFO*> vecDevice;
	//����
	map<string, string> mapParam;
	//���������
	void operator=(CHANNEL_INFO &ci);
private:
	wstring m_ID;
private:
	//����ID
	void GenerateID();
public:
	CHANNEL_INFO();
	//�ͷ���Դ
	~CHANNEL_INFO();

	wstring GetID();
};
