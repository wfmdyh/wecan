#pragma once
#include "DataDef.h"
#include "channel.h"
#include "MyLog.h"

// winsock 2 ��ͷ�ļ��Ϳ�
#include <winsock2.h>
#include <MSWSock.h>
#pragma comment(lib,"ws2_32.lib")

// ���������� (1024*8)
// ֮����Ϊʲô����8K��Ҳ��һ�������ϵľ���ֵ
// ���ȷʵ�ͻ��˷�����ÿ�����ݶ��Ƚ��٣���ô�����õ�СһЩ��ʡ�ڴ�
#define MAX_BUFFER_LEN        8192  


// CIOCPModel��
class CIOCPModel
{
private:
	string m_strIP;						// �������˵�IP��ַ
	int m_nPort;						// �������˵ļ����˿�
	MyLog m_log;						//��־
	CHANNEL_INFO* m_pChannel;			//ͨ������
public:
	fnDataArrive m_DataArrive;			//���ݵ���ʱ���õĻص�����
	SOCKET m_ListenSocket;
	HANDLE m_hShutdownEvent;			// ����֪ͨ�߳�ϵͳ�˳����¼���Ϊ���ܹ����õ��˳��߳�
	
	CRITICAL_SECTION m_csDataFile;		//�����ݰ�д�뵽�ļ��Ļ���
	CRITICAL_SECTION m_csChannel;		//���ӽ��豸�洢��ͨ���Ļ���

	HANDLE m_hThrdAccept;
public:
	CIOCPModel(void);
	~CIOCPModel(void);
	// ����������
	BOOL Start(CHANNEL_INFO* pChannel);
	//	ֹͣ������
	void Stop();
	// ����Socket��
	bool LoadSocketLib();
	void UnloadSocketLib() { WSACleanup(); }

	void SetPort( const int& nPort ) { m_nPort=nPort; }
	int GetPort(){ return m_nPort; }

	//������д���ļ�
	void WriteDataToFile(char *pData, DWORD dwLen);

	//�ȴ���������
	static void ThreadAccept(LPVOID lpParam);
	//�ͻ����߳�
	static void ThreadClient(LPVOID lpThreadParameter);

	// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ�
	bool _IsSocketAlive(SOCKET s);

	//����趨ʱ����û�����ӣ����¶ȳ�ʼΪ-50
	void Init50(time_t NowTime);
	//��ʱ��תΪ�ַ���
	int API_TimeToString(string &strDateStr,time_t &timeData);

	//�����Ӷ�����ã�pDevice������ǽ�������豸����
	void GetOneDevice(CGPRSTemperature *pDevice);
};

