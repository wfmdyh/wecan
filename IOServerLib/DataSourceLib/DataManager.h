/*
���ݹ�����
*/
#pragma once
#include "channel.h"
#include "MyLog.h"

class DataManager
{
private:
	//��Ҫ�洢������
	vector<CHANNEL_INFO*> m_vecChannels;
	MyLog m_Log;
public:
	DataManager();
	~DataManager();
	//����Ҫ�洢������
	void SetChannels(vector<CHANNEL_INFO*> vecChannels);
	//�洢����
	bool SaveData();
};

