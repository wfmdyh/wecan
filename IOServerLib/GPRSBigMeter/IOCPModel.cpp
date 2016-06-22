#include "stdafx.h"
#include "IOCPModel.h"


//线程参数
class CThreadParameter
{
public:
	CIOCPModel *m_model;
	SOCKET m_sockClient;
};

//回调函数，当解析完一个设备以后，连接线程会自动调用
//void CIOCPModel::GetOneDevice(CGPRSBigMeter *pDevice)
//{
//	EnterCriticalSection(&m_csChannel);
//	OutputDebugString(L"GetOneDevice回调\n");
//	//采集的时间戳
//	pDevice->SetLastTime(time(NULL));
//	//添加设备
//	AddMeterToVec(*pDevice);
//	//将收到的连接数据，更新到通道中
//	//vector<DEVICE_INFO*>::iterator ite = m_pChannel->vecDevice.begin();
//	//for (; ite != m_pChannel->vecDevice.end(); ite++)
//	//{
//	//	//找到对应的设备
//	//	DEVICE_INFO *pDev = *ite;
//	//	if (pDev->astrDeviceName.compare(pDevice->GetDeviceID()) == 0)
//	//	{
//	//		//遍历点
//	//		vector<COL_POINT*>::iterator itePoint = pDev->vecPoint.begin();
//	//		for (; itePoint != pDev->vecPoint.end(); itePoint++)
//	//		{
//	//			COL_POINT *pPoint = *itePoint;
//	//			//每次先释放字符串资源
//	//			if (pPoint->varData->vt == VT_BSTR)
//	//			{
//	//				SysFreeString(pPoint->varData->bstrVal);
//	//			}
//	//			if (pPoint->mapParam["301"].compare("状态") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_ALARM]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("累计流量") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_TOTAL_FLOW]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("压力") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_PRESSURE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("压力告警") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_REAL_TIME_ALARM]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("前一天最大瞬时流量") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAXIMUM_INSTANTANEOUS]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("最大时") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAX_I_HOUR]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("最大分") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAX_I_MINUTE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("前一天最小瞬时流量") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MINIMUM_INSTANTANEOUS]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("最小时") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MIN_I_HOUR]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("最小分") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MIN_I_MINUTE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("前一天日期") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_THE_DAY_BEFORE_DATE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("通讯成功率") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_COMMUNICATION_SUCCESS_RATE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("前一天所有压力数据") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_ALL_PRESSURE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("前一天所有温度") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_ALL_TEMPERATURE]));
//	//			}
//	//			
//	//		}
//	//		
//	//		break;
//	//	}
//	//}
//	LeaveCriticalSection(&m_csChannel);
//	//调用IOServer的回调函数
//	//(*m_DataArrive)(m_pChannel);
//	
//	
//}

CIOCPModel::CIOCPModel(void)
{
	// 初始化线程互斥量
	InitializeCriticalSection(&m_csDataFile);
	//InitializeCriticalSection(&m_csChannel);
	//m_hThrdAccept = NULL;
	m_ListenSocket = INVALID_SOCKET;
	
	//m_pChannel = NULL;
	m_DataArrive = NULL;
	//m_vecDevice = NULL;
	m_log.SetFileName(L"大表");
	m_pProtocolMgr = NULL;
}


CIOCPModel::~CIOCPModel(void)
{
	// 确保资源彻底释放
	//this->Stop();
	// 删除客户端列表的互斥量
	DeleteCriticalSection(&m_csDataFile);
	//DeleteCriticalSection(&m_csChannel);

	// 关闭系统退出事件句柄
	RELEASE_HANDLE(m_hShutdownEvent);
}

//将数据写入文件
void CIOCPModel::WriteDataToFile(char *pData, DWORD dwLen)
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
	char szTime[20] = { 0 };
	sprintf_s(szTime, "大表%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
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

//接受请求线程
unsigned int CALLBACK CIOCPModel::ThreadAccept(void *lpParam)
{
	CIOCPModel* pTCP = (CIOCPModel*)lpParam;
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
			int iTimeout = 60000;	//1分钟
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//传给客户端线程的参数
				CThreadParameter * paramThread = new CThreadParameter;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//为每个客户端创建一个线程
				_beginthread(ThreadClient, 0, paramThread);
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
void CIOCPModel::ThreadClient(LPVOID lpParam)
{
	//取出参数
	CThreadParameter *pParam = (CThreadParameter *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	CIOCPModel *pIOCP = pParam->m_model;
	
	//一个连接一个缓存队列
	DeviceConnect *pDevConnect = new DeviceConnect;
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
			//接收到完整的包，尝试解析
			pDevConnect->TryAnalysis();
			//下发指令
			DevCommand cmd;
			if (pIOCP->m_pProtocolMgr->m_CmdOrder.GetOrderById(pDevConnect->m_DeviceID, cmd, SendCommand::DEVICE_TYPE::TYPE_BM))
			{
				cout << "带参数指令\n";
				//cmd.GetCommand(vecCommand, DevCommand::CommandType::PARAM);
			}
			else{
				cout << "不带参数指令\n";
				//cmd.SetDeviceID(pDevConnect->m_DeviceID);
				//cmd.GetCommand(vecCommand, DevCommand::CommandType::NO_PARAM);
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//不完整的包，继续接收

		}else if (nPackageFlag == 3){
			//错误的数据包，断开连接
			//cout << "错误的帧\n";
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		OutputDebugString(L"断开连接\n");
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

//////////////////////////////////////////////////////////////////
//	启动服务器
bool CIOCPModel::Start(CProtocolManager* pPM)
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

	m_hAccept = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept, this, 0, NULL);
	if (m_hAccept == 0)
	{
		cout << "无法创建大表监听线程\n";
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////
//	开始发送系统退出消息，退出完成端口和线程资源
void CIOCPModel::Stop()
{
	// 激活关闭消息通知
	SetEvent(m_hShutdownEvent);
	//等待接受线程退出
	cout << "正在停止大表协议...\n";
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	CloseHandle(m_hAccept);
	m_hAccept = 0;
	
	cout << "大表协议已停止\n";
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

//void CIOCPModel::AddMeterToVec(CGPRSBigMeter meter)
//{
//	EnterCriticalSection(&m_csChannel);
//	//判断是否已经存在
//	vector<CGPRSBigMeter>::iterator iteMeterIsExist = find_if(m_vecMeters.begin(), m_vecMeters.end(), MeterCompare(meter.GetDeviceID()));
//	if (iteMeterIsExist != m_vecMeters.end())
//	{
//		//已经存在
//		*iteMeterIsExist = meter;
//	}
//	else{
//		//不存在
//		m_vecMeters.push_back(meter);
//	}
//	LeaveCriticalSection(&m_csChannel);
//}
//
//vector<CGPRSBigMeter> CIOCPModel::GetMeters()
//{
//	vector<CGPRSBigMeter> tmp;
//	EnterCriticalSection(&m_csChannel);
//	tmp = m_vecMeters;
//	LeaveCriticalSection(&m_csChannel);
//	return tmp;
//}