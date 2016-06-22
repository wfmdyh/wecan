#pragma once

class CHANNEL_INFO;
class TCPModel;
class DataManager;
class DevPipeNet;

class ProtocolMgr
{
private:
	const string PROTOCOL_NAME;
	list<DevPipeNet*> m_ArrDevice;				//实时数据，每次存储进数据库以后，自动清空
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
	//删除所有设备，释放资源
	void DeleteAllDevice();
	list<DevPipeNet*> GetAllDevice();
	//数据库线程
	static DWORD WINAPI ThreadDatabase(_In_ LPVOID lpParameter);
};

