/*
������TCPģ��
*/
#pragma once

class TCPModelConcentrator
{
private:
	HANDLE m_hAccept;
public:
	int m_nPort;						// �������˵ļ����˿�
	CProtocolManager* m_pProtocolMgr;
	HANDLE m_hShutdownEvent;
	SOCKET m_ListenSocket;
	MyLog m_log;
	CRITICAL_SECTION m_csDataFile;		//�����ݰ�д�뵽�ļ��Ļ���
	SendCommand m_CmdOrder;
public:
	TCPModelConcentrator();
	~TCPModelConcentrator();

	// ����������
	bool Start(CProtocolManager* pPM);
	//	ֹͣ������
	void Stop();
	//������д���ļ�
	void WriteDataToFile(char *pData, DWORD dwLen);
	//�ͻ����߳�
	static void ThreadClient2(LPVOID lpThreadParameter);
	//���������߳�
	static unsigned int CALLBACK ThreadAccept2(void *lpParam);
};

