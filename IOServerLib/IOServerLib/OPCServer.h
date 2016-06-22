#pragma once
#include "TransmitManager.h"
#include "OPCTag.h"

class COPCServer
{
private:
	TransmitManager *m_TraMgr;		//转发通道的配置信息
	int m_nUpdataTime;				//更新时间
	vector<OPCTag*> m_vecTags;
public:
	COPCServer();
	~COPCServer();
	static BOOL InitServer();
	//注册函数
	static BOOL RegOPCServer(LPCTSTR wszAppExePath);
	//反注册
	static BOOL UnRegOPCServer();

	BOOL Start();
	void Stop();
	//设置转发通道
	void SetTransmit(TransmitManager *pTransmitMgr);
	//转发值
	BOOL TransmitData();
private:
	BOOL CreateTags();
};
