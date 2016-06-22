#include "stdafx.h"
#include "GPRSTemperature.h"
#include "IOCPModel.h"
#include "DeviceConnect.h"

//recv的大小
#define DEFAULT_BUFLEN 1024

//缓存大小
#define DATA_BUF_SIZE					(1024*1024*10)

// 释放句柄宏
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}

//线程参数
class CThreadParameter
{
public:
	CIOCPModel *m_model;
	SOCKET m_sockClient;
};

//回调函数，当解析完一个设备以后，连接线程会自动调用
void CIOCPModel::GetOneDevice(CGPRSTemperature *pDevice)
{
	EnterCriticalSection(&m_csChannel);
	OutputDebugString(L"GetOneDevice回调\n");
	//将收到的连接数据，更新到通道中
	for (unsigned i = 0; i < m_pChannel->vecDevice.size(); i++)
	{
		//找到对应的设备
		DEVICE_INFO *pDev = m_pChannel->vecDevice.at(i);
		if (pDev->astrDeviceName.compare(pDevice->GetDeviceID()) == 0)
		{
			//只取第一个
			COL_POINT *pPoint = pDev->vecPoint.at(0);
			VariantCopy(pPoint->varData, &(pDevice->m_values[RI_Temperature]));
			//WF 3-10
			pPoint->m_IsChange = true;
			break;
		}
	}
	LeaveCriticalSection(&m_csChannel);
	//调用IOServer的回调函数
	(*m_DataArrive)(m_pChannel);
	
	
}

CIOCPModel::CIOCPModel(void)
{
	m_hThrdAccept = NULL;
	m_ListenSocket = INVALID_SOCKET;
	
	m_DataArrive = NULL;
	//m_vecDevice = NULL;
	m_log.SetFileName(L"室温");
}


CIOCPModel::~CIOCPModel(void)
{
	// 确保资源彻底释放
	//this->Stop();
}

//将数据写入文件
void CIOCPModel::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	string strFilePath = "C://IOServer日志//";
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
	char szTime[20] = { 0 };
	sprintf_s(szTime, "室温%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
	strFilePath += szTime;
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
	memset(szTime, 0, 20);
	sprintf_s(szTime, "\r\n%02d:%02d:%02d:", st.wHour, st.wMinute, st.wSecond);
	fwrite(szTime, 1, strlen(szTime), fe);
	if (fwrite(pData, dwLen, 1, fe) < 1)
	{
		m_log.LogMsgToFile(L"写入文件失败：室温数据");
		fclose(fe);
		LeaveCriticalSection(&m_csDataFile);
		return;
	}

	fclose(fe);
	LeaveCriticalSection(&m_csDataFile);
}

//====================================================================================
//
//				    系统初始化和终止
//
//====================================================================================




////////////////////////////////////////////////////////////////////
// 初始化WinSock 2.2
bool CIOCPModel::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 错误(一般都不可能出现)
	if (NO_ERROR != nResult)
	{
		m_log.LogMsgToFile(L"初始化WinSock 2.2失败！");
		return false;
	}

	return true;
}

//扫描连接线程
void ScanConn(void *arg)
{
	CIOCPModel * pIOCP = (CIOCPModel*)arg;
	time_t dTimer = NULL,LastTimer=NULL;
	time(&dTimer);
	while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCP->m_hShutdownEvent, 0))
	{
		if (!LastTimer)
		{
			LastTimer = dTimer;
		}
		if (dTimer - LastTimer > 10)
		{
			LastTimer = dTimer;
			pIOCP->Init50(dTimer);
		   
		}
		else
		{
			time(&dTimer);
		}
	}
}

//客户端线程
void CIOCPModel::ThreadClient(LPVOID lpParam)
{
	OutputDebugString(L"进入客户端线程\n");
	//取出参数
	CThreadParameter *pParam = (CThreadParameter *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	CIOCPModel *pIOCP = pParam->m_model;

	//一个连接一个缓存队列
	DeviceConnect *pDevConnect = new DeviceConnect;
	//用于线程同步
	pDevConnect->m_Parent = pIOCP;
	//传递回调函数
	//pDevConnect->m_GetOneDevice = GetOneDevice;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	while (iResult > 0)
	{
		//写数据到文件
		pIOCP->WriteDataToFile(recvbuf, iResult);
		pDevConnect->CopyDataToBuf(recvbuf, iResult);
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		OutputDebugString(L"断开连接\n");
	}
	else{
		//printf("recv failed with error: %d\n", WSAGetLastError());
		OutputDebugString(L"recv 失败\n");
	}
	
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	// cleanup
	closesocket(ClientSocket);
	delete pDevConnect;
	pDevConnect = NULL;
	_endthread();
	OutputDebugString(L"离开客户端线程\n");
}

//accept线程
void CIOCPModel::ThreadAccept(LPVOID lpThreadParameter)
{
	CIOCPModel* pIOCP = (CIOCPModel*)lpThreadParameter;
	SOCKET ListenSocket = pIOCP->m_ListenSocket;
	while (WAIT_OBJECT_0 != WaitForSingleObject(pIOCP->m_hShutdownEvent, 0))
	{
		SOCKET ClientSocket = INVALID_SOCKET;
		// Accept a client socket
		ClientSocket = accept(ListenSocket, NULL, NULL);
		if (ClientSocket == INVALID_SOCKET) 
		{
			pIOCP->m_log.LogMsgToFile(L"accept failed with error: %d", WSAGetLastError());
		}
		else
		{
			//设置超时时间
			int iTimeout = 5000;	//3s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				/*THREADPARAMS_CLIENT* tc = new THREADPARAMS_CLIENT;
				tc->pIOCP = pIOCP;
				tc->sockClient = ClientSocket;*/
				//传给客户端线程的参数
				CThreadParameter * paramThread = new CThreadParameter;
				paramThread->m_model = pIOCP;
				paramThread->m_sockClient = ClientSocket;
				//为每个客户端创建一个线程
				_beginthread(ThreadClient, 0, paramThread);
			}
			else{
				pIOCP->m_log.LogMsgToFile(L"setsockopt 失败");
				closesocket(ClientSocket);
			}
		}
	}
	WSACleanup();
	_endthread();
}

//////////////////////////////////////////////////////////////////
//	启动服务器
BOOL CIOCPModel::Start(CHANNEL_INFO* pChannel)
{
	m_pChannel = pChannel;
	// 初始化线程互斥量
	InitializeCriticalSection(&m_csDataFile);
	InitializeCriticalSection(&m_csChannel);

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
		return FALSE;
	}

	m_ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (m_ListenSocket == INVALID_SOCKET) {
		m_log.LogMsgToFile(L"socket failed with error: %ld", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return FALSE;
	}

	// Setup the TCP listening socket
	iResult = bind(m_ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		m_log.LogMsgToFile(L"bind failed with error: %d", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	freeaddrinfo(result);

	iResult = listen(m_ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		m_log.LogMsgToFile(L"listen failed with error: %d", WSAGetLastError());
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	//创建accept线程
	m_hThrdAccept = (HANDLE)_beginthread(ThreadAccept, 0, this);
	if (m_hThrdAccept == NULL)
	{
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	OutputDebugString(L"系统准备就绪，等候连接....\n");

	return TRUE;
}


////////////////////////////////////////////////////////////////////
//	开始发送系统退出消息，退出完成端口和线程资源
void CIOCPModel::Stop()
{
		// 激活关闭消息通知
		SetEvent(m_hShutdownEvent);

		//// 等待所有的客户端资源退出
		//WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);
		closesocket(m_ListenSocket);
		WSACleanup();
		TerminateThread(m_hThrdAccept, 0);
	

		// 删除客户端列表的互斥量
		DeleteCriticalSection(&m_csDataFile);
		DeleteCriticalSection(&m_csChannel);

		// 关闭系统退出事件句柄
		RELEASE_HANDLE(m_hShutdownEvent);

		//this->_ShowMessage("停止监听\n");
}

/////////////////////////////////////////////////////////////////////
// 判断客户端Socket是否已经断开，否则在一个无效的Socket上投递WSARecv操作会出现异常
// 使用的方法是尝试向这个socket发送数据，判断这个socket调用的返回值
// 因为如果客户端网络异常断开(例如客户端崩溃或者拔掉网线等)的时候，服务器端是无法收到客户端断开的通知的

bool CIOCPModel::_IsSocketAlive(SOCKET s)
{
	int nByteSent = send(s, "", 0, 0);
	if (-1 == nByteSent) return false;
	return true;
}

//采集时间大于设定时间，就给初始值
void CIOCPModel::Init50(time_t NowTime)
{
	//map<string, CGPRSTemperature*>::iterator it;
	//CGPRSTemperature *p_temp;

	//for (it = (*m_vecDevice).begin(); it != (*m_vecDevice).end() && m_uiDeviceCount > 0; ++it)
	//{
	//	p_temp = it->second;
	//	if (NULL == p_temp)
	//		continue;

	//	if (difftime(NowTime, p_temp->LastTime) > 7200)// 7200秒，大于两个小时就视为故障
	//	{
	//		(*m_vecDevice)[it->first]->m_values[RI_Temperature].vt = VT_R4;
	//		(*m_vecDevice)[it->first]->m_values[RI_Temperature].fltVal = -50.0;
	//	}

	//}

}


int CIOCPModel::API_TimeToString(string &strDateStr, time_t &timeData)
{
	char chTmp[30];
	memset(chTmp, '0', sizeof(chTmp));
	struct tm p = {0};
	localtime_s(&p, &timeData);
	p.tm_year = p.tm_year + 1900;
	p.tm_mon = p.tm_mon + 1;

	sprintf_s(chTmp, "%04d-%02d-%02d %02d:%02d:%02d", p.tm_year, p.tm_mon, p.tm_mday, p.tm_hour, p.tm_min, p.tm_sec);

	strDateStr = chTmp;
	return 1;
}