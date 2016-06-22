/*
数据管理类
wfm
2016-4-14
*/
#pragma once
class DataManager
{
private:
	vector<CHANNEL_INFO> m_Channels;
	CRITICAL_SECTION m_LockChannel;
public:
	DataManager();
	~DataManager();
	//添加一个通道数据
	BOOL AddChannel(CHANNEL_INFO channel);
	//获取通道的监听端口
	int GetChannelPort(string strName);

	void Lock();
	void Unlock();
};

