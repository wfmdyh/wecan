#pragma once
class ProtocolMgr;
class CommandMgr;
class TCPModel
{
public:
	//MyLog* m_log;
	int m_nPort;						// �������˵ļ����˿�
	//HANDLE m_hShutdownEvent;
	SOCKET m_ListenSocket;
	ProtocolMgr* m_pProtocolMgr;
	HANDLE m_hAccept;
	CRITICAL_SECTION m_csDataFile;
	MyTools m_tools;
	CommandMgr* m_pCmdMgr;
public:
	TCPModel();
	~TCPModel();
	//��ʼ��winsock��
	BOOL LoadSocketLib();

	// ����������
	BOOL Start(ProtocolMgr* pPM);
	//	ֹͣ������
	void Stop();
	//������д���ļ�
	void WriteDataToFile(char *pData, DWORD dwLen);
	//�ͻ����߳�
	static void ThreadClient(LPVOID lpThreadParameter);
	//���������߳�
	static unsigned int CALLBACK ThreadAccept(void *lpParam);
};

