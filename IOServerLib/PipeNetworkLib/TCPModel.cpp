#include "stdafx.h"
#include "MyLog.h"
#include "ProtocolMgr.h"
#include "TCPModel.h"
#include "DataDef.h"
#include "DeviceConnect.h"
#include "MyTools.h"
#include "DevPipeNet.h"
#include "DevCommand.h"
#include "CommandMgr.h"

////////////////////
struct THREAD_PARAM
{
public:
	TCPModel *m_model;
	SOCKET m_sockClient;
	THREAD_PARAM()
	{
		m_model = NULL;
		m_sockClient = INVALID_SOCKET;
	}
};
//////////////////////


TCPModel::TCPModel()
{
	//m_log = new MyLog;
	/*m_log.SetFileName(L"水表集中器");*/
	InitializeCriticalSection(&m_csDataFile);
	m_hAccept = NULL;
	m_pProtocolMgr = NULL;
	m_nPort = 0;
	m_ListenSocket = INVALID_SOCKET;
	m_pCmdMgr = new CommandMgr();
}


TCPModel::~TCPModel()
{
	DeleteCriticalSection(&m_csDataFile);
	//delete m_log;
	delete m_pCmdMgr;
}

BOOL TCPModel::Start(ProtocolMgr* pPM)
{
	wstringstream wss;
	wstring strMsg;

	if (!LoadSocketLib())
	{
		ShowMessage(L"初始化winsock库失败，请检查操作系统是否设置正确");
		return FALSE;
	}
	m_pProtocolMgr = pPM;
	

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
		wss.clear();
		wss << L"getaddrinfo failed with error: " << iResult;
		strMsg = wss.str();
		ShowMessage(strMsg.c_str());
		WSACleanup();
		return FALSE;
	}

	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ListenSocket == INVALID_SOCKET) {
		wss.clear();
		wss << L"socket failed with error: " << WSAGetLastError();
		strMsg = wss.str();
		ShowMessage(strMsg.c_str());
		freeaddrinfo(result);
		WSACleanup();
		return FALSE;
	}

	// Setup the TCP listening socket
	iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		wss.clear();
		wss << L"bind failed with error: " << WSAGetLastError();
		strMsg = wss.str();
		ShowMessage(strMsg.c_str());
		freeaddrinfo(result);
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	freeaddrinfo(result);

	iResult = listen(m_ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		wss.clear();
		wss << L"listen failed with error: " << WSAGetLastError();
		strMsg = wss.str();
		ShowMessage(strMsg.c_str());
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	m_hAccept = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept, this, 0, NULL);
	if (m_hAccept == 0)
	{
		ShowMessage(L"无法创建大表监听线程");
		return FALSE;
	}
	return TRUE;
}

void TCPModel::Stop()
{
	
	//等待接受线程退出
	ShowMessage(L"正在停止协议...");
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	if (m_hAccept != 0)
	{
		CloseHandle(m_hAccept);
	}
	m_hAccept = 0;

	ShowMessage(L"协议已停止");
}


//接受请求线程
unsigned int CALLBACK TCPModel::ThreadAccept(void *lpParam)
{
	wstringstream wss;
	wstring strMsg;
	TCPModel* pTCP = (TCPModel*)lpParam;
	//客户端线程句柄
	list<HANDLE> hClients;
	
	while (WAIT_OBJECT_0 != WaitForSingleObject(pTCP->m_pProtocolMgr->m_hShutdownEvent, 0))
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		// Accept a client socket
		ClientSocket = accept(pTCP->m_ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET)
		{
			wss.clear();
			wss << L"accept failed with error: " << WSAGetLastError();
			strMsg = wss.str();
			ShowMessage(strMsg.c_str());
		}
		else
		{
			//设置超时时间
			int iTimeout = 5000;	//5s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//传给客户端线程的参数
				THREAD_PARAM * paramThread = new THREAD_PARAM;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//为每个客户端创建一个线程
				_beginthread(ThreadClient, 0, paramThread);
			}
			else{
				ShowMessage(L"setsockopt 失败");
				closesocket(ClientSocket);
			}
		}
	}
	return 0;
}

//客户端线程
void TCPModel::ThreadClient(LPVOID lpParam)
{
	wstringstream wss;
	wstring strMsg;

	////取出参数
	THREAD_PARAM *pParam = (THREAD_PARAM *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	TCPModel *pIOCP = pParam->m_model;

	//一个连接一个缓存队列
	DeviceConnect *pDevConnect = new(nothrow)DeviceConnect;
	//用于线程同步
	//pDevConnect->m_Parent = pIOCP;
	const int recvbuflen = 1024;
	char recvbuf[recvbuflen] = { 0 };
	

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
			//进行赋值
			pIOCP->m_pProtocolMgr->UpdateDevice(pDevConnect->m_Device);
			//下发指令
			DevCommand cmd;
			pIOCP->m_pCmdMgr->GetCommandByID(pDevConnect->m_DeviceID, cmd);
			if (cmd.GetCmdType() == DevCommand::PARAM)
			{
				ShowMessage(L"多参数，带参数指令");
			}
			else{
				ShowMessage(L"多参数，不带参数指令");
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//不完整的包，继续接收
			ShowMessage(L"不完整的包，等待继续接收");
		}
		else if (nPackageFlag == 3){
			//错误的数据包，断开连接
			ShowMessage(L"错误的帧");
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		ShowMessage(L"断开连接");
	}
	else{
		wss.clear();
		wss << L"recv 失败，错误码：" << WSAGetLastError();
		strMsg = wss.str();
		ShowMessage(strMsg.c_str());
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
void TCPModel::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	wstringstream wss;
	wstring strMsg;

	string strFilePath = "C:\\IOServer日志\\";
	//判断文件是否存在
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//目录不存在，创建目录
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			LeaveCriticalSection(&m_csDataFile);
			ShowMessage(L"无法创建日志文件");
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	string strFileName;
	stringstream ss;
	
	ss << "多参数_" << st.wYear << "_";
	ss << setw(2) << setfill('0') << st.wMonth;
	ss << "_";
	ss << setw(2) << setfill('0') << st.wDay;
	ss << ".data";
	strFileName = ss.str();
	strFilePath += strFileName;
	//追加只写
	FILE *fe = NULL;
	fopen_s(&fe, strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		ShowMessage(L"打开文件失败：多参数");
		LeaveCriticalSection(&m_csDataFile);
		return;
	}
	//写入时间
	string strTime;
	ss.clear();
	ss.str("");
	ss << "\r\n";
	ss << setw(2) << setfill('0') << st.wHour;
	ss << ":";
	ss << setw(2) << setfill('0') << st.wMinute;
	ss << ":";
	ss << setw(2) << setfill('0') << st.wSecond;
	ss << ":";
	strTime = ss.str();
	fwrite(strTime.c_str(), 1, strTime.size(), fe);
	//转换数据格式
	string strHexData = m_tools.ToHexStr(pData, dwLen);
	if (fwrite(strHexData.c_str(), strHexData.size(), 1, fe) < 1)
	{
		ShowMessage(L"写入文件失败：多参数");
		fclose(fe);
		LeaveCriticalSection(&m_csDataFile);
		return;
	}

	fclose(fe);
	LeaveCriticalSection(&m_csDataFile);
}

BOOL TCPModel::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (NO_ERROR != nResult)
	{
		return FALSE;
	}
	return TRUE;
}