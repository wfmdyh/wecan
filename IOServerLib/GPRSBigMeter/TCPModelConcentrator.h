/*
集中器TCP模型
*/
#pragma once

class TCPModelConcentrator
{
private:
	HANDLE m_hAccept;
public:
	int m_nPort;						// 服务器端的监听端口
	CProtocolManager* m_pProtocolMgr;
	HANDLE m_hShutdownEvent;
	SOCKET m_ListenSocket;
	MyLog m_log;
	CRITICAL_SECTION m_csDataFile;		//将数据包写入到文件的互斥
	SendCommand m_CmdOrder;
public:
	TCPModelConcentrator();
	~TCPModelConcentrator();

	// 启动服务器
	bool Start(CProtocolManager* pPM);
	//	停止服务器
	void Stop();
	//将数据写入文件
	void WriteDataToFile(char *pData, DWORD dwLen);
	//客户端线程
	static void ThreadClient2(LPVOID lpThreadParameter);
	//接受请求线程
	static unsigned int CALLBACK ThreadAccept2(void *lpParam);
};

