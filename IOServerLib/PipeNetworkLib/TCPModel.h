#pragma once
class ProtocolMgr;
class CommandMgr;
class TCPModel
{
public:
	//MyLog* m_log;
	int m_nPort;						// 服务器端的监听端口
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
	//初始化winsock库
	BOOL LoadSocketLib();

	// 启动服务器
	BOOL Start(ProtocolMgr* pPM);
	//	停止服务器
	void Stop();
	//将数据写入文件
	void WriteDataToFile(char *pData, DWORD dwLen);
	//客户端线程
	static void ThreadClient(LPVOID lpThreadParameter);
	//接受请求线程
	static unsigned int CALLBACK ThreadAccept(void *lpParam);
};

