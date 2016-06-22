#pragma once
//#include "ProtocolManager.h"

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#define MAX_BUFFER_LEN        8192  

class CProtocolManager;
// CIOCPModel��
class CIOCPModel
{
private:
	string m_strIP;						// �������˵�IP��ַ
	int m_nPort;						// �������˵ļ����˿�
	MyLog m_log;						//��־
	HANDLE m_hAccept;
	//vector<CGPRSBigMeter> m_vecMeters;	//�豸����
public:
	fnDataArrive m_DataArrive;			//���ݵ���ʱ���õĻص�����
	SOCKET m_ListenSocket;
	HANDLE m_hShutdownEvent;			// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�
	
	CRITICAL_SECTION m_csDataFile;		//�����ݰ�д�뵽�ļ��Ļ���
	//CRITICAL_SECTION m_csChannel;		//���ӽ��豸�洢��ͨ���Ļ���

	
	CProtocolManager *m_pProtocolMgr;	//Э�������
public:
	CIOCPModel(void);
	~CIOCPModel(void);

	//void AddMeterToVec(CGPRSBigMeter meter);
	//vector<CGPRSBigMeter> GetMeters();

	// ����������
	bool Start(CProtocolManager* pPM);
	//	ֹͣ������
	void Stop();

	void SetPort( const int& nPort ) { m_nPort=nPort; }
	int GetPort(){ return m_nPort; }
	//������д���ļ�
	void WriteDataToFile(char *pData, DWORD dwLen);
	//�ͻ����߳�
	static void ThreadClient(LPVOID lpThreadParameter);
	//���������߳�
	static unsigned int CALLBACK ThreadAccept(void *lpParam);
	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	bool _IsSocketAlive(SOCKET s);
	//�����Ӷ�����ã�pDevice������ǽ�������豸����
	//void GetOneDevice(CGPRSBigMeter *pDevice);
};

//�ж�ID�Ƿ����
class MeterCompare
{
private:
	string m_strID;
public:
	MeterCompare(string strID)
	{
		m_strID = strID;
	}
	bool operator()(vector<CGPRSBigMeter>::value_type value)
	{
		return (value.GetDeviceID().compare(m_strID) == 0);
	}
};

//class ConcentratorCompare
//{
//private:
//	DWORD m_ID;
//public:
//	ConcentratorCompare(DWORD dwID)
//	{
//		m_ID = dwID;
//	}
//	bool operator()(vector<DevConcentrator>::value_type value)
//	{
//		return (value.m_DeviceID == m_ID);
//	}
//};