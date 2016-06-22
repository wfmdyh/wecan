/*
采集点管理类，用于数据之间多线程的互斥
可改进，将转发通道和采集通道分开处理，在IOServer启动多个线程，驱动动态库中只传入副本，减少并发
*/
#pragma once
#include "TransmitChannel.h"
#include "channel.h"
#include "MyLog.h"

class TransmitManager
{
private:
	vector<TransmitChannel*> m_vecChannel;				//转通道
	HANDLE m_hOPC;										//opc server的线程句柄
	MyLog m_log;										//日志
public:
	TransmitManager();
	~TransmitManager();
	void AddTransmitChannel(TransmitChannel *pChannel);
	vector<TransmitChannel*> GetVecChannel();
	//获取指点协议的所有通道，注意要释放资源
	TransmitManager* GetProtocolChannel(string strProtocolName);
	//转发点关联上一个采集通道坐标
	void RelevanceColIndex(vector<CHANNEL_INFO*> vecChannel);
	
	//开始
	bool Start();
	//停止
	bool Stop();
	//打印数据
	void ShowData();
};

