/*
数据管理类
*/
#pragma once
#include "channel.h"
#include "MyLog.h"

class DataManager
{
private:
	//需要存储的数据
	vector<CHANNEL_INFO*> m_vecChannels;
	MyLog m_Log;
public:
	DataManager();
	~DataManager();
	//设置要存储的数据
	void SetChannels(vector<CHANNEL_INFO*> vecChannels);
	//存储数据
	bool SaveData();
};

