#include "stdafx.h"
#include "TCPModelConcentrator.h"


class CThreadParameter2
{
public:
	TCPModelConcentrator *m_model;
	SOCKET m_sockClient;
};

TCPModelConcentrator::TCPModelConcentrator()
{
	m_log.SetFileName(L"水表集中器");
	m_nPort = 0;
	m_ListenSocket = INVALID_SOCKET;
	m_pProtocolMgr = NULL;
	m_hAccept = 0;
	InitializeCriticalSection(&m_csDataFile);
}


TCPModelConcentrator::~TCPModelConcentrator()
{
	DeleteCriticalSection(&m_csDataFile);
}

bool TCPModelConcentrator::Start(CProtocolManager* pPM)
{
	m_pProtocolMgr = pPM;
	// 建立系统退出的事件通知
	m_hShutdownEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	char szPort[10] = { 0 };
	sprintf_s(szPort, "%d", m_nPort);
	int iResult = getaddrinfo(NULL, szPort, &hints, &result);
	if (iResult != 0) {
		m_log.LogMsgToFile(L"getaddrinfo failed with error: %d", iResult);
		WSACleanup();
		return false;
	}

	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ListenSocket == INVALID_SOCKET) {
		m_log.LogMsgToFile(L"socket failed with error: %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}

	// Setup the TCP listening socket
	iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		m_log.LogMsgToFile(L"bind failed with error: %d", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(m_ListenSocket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(result);

	iResult = listen(m_ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		m_log.LogMsgToFile(L"listen failed with error: %d", WSAGetLastError());
		closesocket(m_ListenSocket);
		WSACleanup();
		return false;
	}

	m_hAccept = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept2, this, 0, NULL);
	if (m_hAccept == 0)
	{
		cout << "无法创建大表监听线程\n";
		return false;
	}
	return true;
}

void TCPModelConcentrator::Stop()
{
	// 激活关闭消息通知
	SetEvent(m_hShutdownEvent);
	//等待接受线程退出
	cout << "正在停止集中器协议...\n";
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	if (m_hAccept != 0)
	{
		CloseHandle(m_hAccept);
	}
	m_hAccept = 0;

	cout << "集中器协议已停止\n";
}


//接受请求线程
unsigned int CALLBACK TCPModelConcentrator::ThreadAccept2(void *lpParam)
{
	TCPModelConcentrator* pTCP = (TCPModelConcentrator*)lpParam;
	//客户端线程句柄
	list<HANDLE> hClients;

	while (WAIT_OBJECT_0 != WaitForSingleObject(pTCP->m_hShutdownEvent, 0))
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		// Accept a client socket
		ClientSocket = accept(pTCP->m_ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			pTCP->m_log.LogMsgToFile(L"accept failed with error: %d", WSAGetLastError());
		}
		else
		{
			//设置超时时间
			int iTimeout = 60000;	//60s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//传给客户端线程的参数
				CThreadParameter2 * paramThread = new CThreadParameter2;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//为每个客户端创建一个线程
				_beginthread(ThreadClient2, 0, paramThread);
			}
			else{
				pTCP->m_log.LogMsgToFile(L"setsockopt 失败");
				closesocket(ClientSocket);
			}
		}
	}
	return 0;
}

//客户端线程
void TCPModelConcentrator::ThreadClient2(LPVOID lpParam)
{
	//取出参数
	CThreadParameter2 *pParam = (CThreadParameter2 *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	TCPModelConcentrator *pIOCP = pParam->m_model;

	//一个连接一个缓存队列
	DeviceConnectCon *pDevConnect = new(nothrow) DeviceConnectCon;
	//用于线程同步
	pDevConnect->m_Parent = pIOCP;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	while (iResult > 0)
	{
		//写数据到文件
		pIOCP->WriteDataToFile(recvbuf, iResult);
		int nPackageFlag = pDevConnect->CopyDataToBuf(recvbuf, iResult);
		if (nPackageFlag == 1)
		{
			//接收完所有的帧，尝试解析
			pDevConnect->TryAnalysis();
			//下发指令
			DevCommand cmd;
			if (pIOCP->m_CmdOrder.GetOrderById(pDevConnect->m_DeviceID, cmd, SendCommand::DEVICE_TYPE::TYPE_CON))
			{
				cout << "集中器，带参数指令\n";
			}
			else{
				cout << "集中器，不带参数指令\n";
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//不完整的包，继续接收
			cout << "不完整的包，等待继续接收" << endl;
		}
		else if (nPackageFlag == 3){
			//错误的数据包，断开连接
			cout << "错误的帧\n";
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		cout << "断开连接\n";
	}
	else{
		cout << "recv 失败，错误码：" << WSAGetLastError() << endl;
	}
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	// cleanup
	closesocket(ClientSocket);
	delete pDevConnect;
	pDevConnect = NULL;
	_endthread();
}

//将数据写入文件
void TCPModelConcentrator::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	string strFilePath = "C:\\IOServer日志\\";
	//判断文件是否存在
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//目录不存在，创建目录
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			LeaveCriticalSection(&m_csDataFile);
			MessageBox(NULL, L"无法创建日志文件", L"错误", MB_OK | MB_ICONERROR);
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	string strFileName;
	stringstream ss;
	ss << "集中器_" << st.wYear << "_" << st.wMonth << "_" << st.wDay << ".data";
	strFileName = ss.str();
	strFilePath += strFileName;
	//追加只写
	FILE *fe = NULL;
	fopen_s(&fe, strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		m_log.LogMsgToFile(L"打开文件失败：室温数据");
		LeaveCriticalSection(&m_csDataFile);
		return;
	}
	//写入时间
	string strTime;
	ss.clear();
	ss.str("");
	ss << "\r\n" << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":";
	strTime = ss.str();
	fwrite(strTime.c_str(), 1, strTime.size(), fe);
	//转换数据格式
	string strHexData = ToHexStr(pData, dwLen);
	if (fwrite(strHexData.c_str(), strHexData.size(), 1, fe) < 1)
	{
		m_log.LogMsgToFile(L"写入文件失败：室温数据");
		fclose(fe);
		LeaveCriticalSection(&m_csDataFile);
		return;
	}

	fclose(fe);
	LeaveCriticalSection(&m_csDataFile);
}