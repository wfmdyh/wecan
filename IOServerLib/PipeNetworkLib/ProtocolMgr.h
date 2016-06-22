#pragma once

class CHANNEL_INFO;
class TCPModel;
class DataManager;
class DevPipeNet;

class ProtocolMgr
{
private:
	const string PROTOCOL_NAME;
	list<DevPipeNet*> m_ArrDevice;				//ʵʱ���ݣ�ÿ�δ洢�����ݿ��Ժ��Զ����
	CRITICAL_SECTION m_csDevice;
	HANDLE m_hDatabase;
public:
	int m_nBigMeterPort;
	TCPModel* m_TcpModel;
	HANDLE m_hShutdownEvent;
public:
	ProtocolMgr();
	~ProtocolMgr();
	BOOL Start(DataManager* pMgr);
	void Stop();
	void UpdateDevice(DevPipeNet* dev);
	//ɾ�������豸���ͷ���Դ
	void DeleteAllDevice();
	list<DevPipeNet*> GetAllDevice();
	//���ݿ��߳�
	static DWORD WINAPI ThreadDatabase(_In_ LPVOID lpParameter);
};

