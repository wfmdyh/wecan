#pragma once
#include "TransmitManager.h"
#include "OPCTag.h"

class COPCServer
{
private:
	TransmitManager *m_TraMgr;		//ת��ͨ����������Ϣ
	int m_nUpdataTime;				//����ʱ��
	vector<OPCTag*> m_vecTags;
public:
	COPCServer();
	~COPCServer();
	static BOOL InitServer();
	//ע�ắ��
	static BOOL RegOPCServer(LPCTSTR wszAppExePath);
	//��ע��
	static BOOL UnRegOPCServer();

	BOOL Start();
	void Stop();
	//����ת��ͨ��
	void SetTransmit(TransmitManager *pTransmitMgr);
	//ת��ֵ
	BOOL TransmitData();
private:
	BOOL CreateTags();
};
