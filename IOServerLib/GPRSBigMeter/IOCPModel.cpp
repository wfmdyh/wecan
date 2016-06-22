#include "stdafx.h"
#include "IOCPModel.h"


//�̲߳���
class CThreadParameter
{
public:
	CIOCPModel *m_model;
	SOCKET m_sockClient;
};

//�ص���������������һ���豸�Ժ������̻߳��Զ�����
//void CIOCPModel::GetOneDevice(CGPRSBigMeter *pDevice)
//{
//	EnterCriticalSection(&m_csChannel);
//	OutputDebugString(L"GetOneDevice�ص�\n");
//	//�ɼ���ʱ���
//	pDevice->SetLastTime(time(NULL));
//	//����豸
//	AddMeterToVec(*pDevice);
//	//���յ����������ݣ����µ�ͨ����
//	//vector<DEVICE_INFO*>::iterator ite = m_pChannel->vecDevice.begin();
//	//for (; ite != m_pChannel->vecDevice.end(); ite++)
//	//{
//	//	//�ҵ���Ӧ���豸
//	//	DEVICE_INFO *pDev = *ite;
//	//	if (pDev->astrDeviceName.compare(pDevice->GetDeviceID()) == 0)
//	//	{
//	//		//������
//	//		vector<COL_POINT*>::iterator itePoint = pDev->vecPoint.begin();
//	//		for (; itePoint != pDev->vecPoint.end(); itePoint++)
//	//		{
//	//			COL_POINT *pPoint = *itePoint;
//	//			//ÿ�����ͷ��ַ�����Դ
//	//			if (pPoint->varData->vt == VT_BSTR)
//	//			{
//	//				SysFreeString(pPoint->varData->bstrVal);
//	//			}
//	//			if (pPoint->mapParam["301"].compare("״̬") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_ALARM]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("�ۼ�����") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_TOTAL_FLOW]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ѹ��") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_CURRENT_PRESSURE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ѹ���澯") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_REAL_TIME_ALARM]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ǰһ�����˲ʱ����") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAXIMUM_INSTANTANEOUS]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("���ʱ") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAX_I_HOUR]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("����") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MAX_I_MINUTE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ǰһ����С˲ʱ����") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MINIMUM_INSTANTANEOUS]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("��Сʱ") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MIN_I_HOUR]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("��С��") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_MIN_I_MINUTE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ǰһ������") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_THE_DAY_BEFORE_DATE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ͨѶ�ɹ���") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_COMMUNICATION_SUCCESS_RATE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ǰһ������ѹ������") == 0)
//	//			{
//	//				VariantCopy(pPoint->varData, &(pDevice->m_values[RI_ALL_PRESSURE]));
//	//			}
//	//			else if (pPoint->mapParam["301"].compare("ǰһ�������¶�") == 0)
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
//	//����IOServer�Ļص�����
//	//(*m_DataArrive)(m_pChannel);
//	
//	
//}

CIOCPModel::CIOCPModel(void)
{
	// ��ʼ���̻߳�����
	InitializeCriticalSection(&m_csDataFile);
	//InitializeCriticalSection(&m_csChannel);
	//m_hThrdAccept = NULL;
	m_ListenSocket = INVALID_SOCKET;
	
	//m_pChannel = NULL;
	m_DataArrive = NULL;
	//m_vecDevice = NULL;
	m_log.SetFileName(L"���");
	m_pProtocolMgr = NULL;
}


CIOCPModel::~CIOCPModel(void)
{
	// ȷ����Դ�����ͷ�
	//this->Stop();
	// ɾ���ͻ����б�Ļ�����
	DeleteCriticalSection(&m_csDataFile);
	//DeleteCriticalSection(&m_csChannel);

	// �ر�ϵͳ�˳��¼����
	RELEASE_HANDLE(m_hShutdownEvent);
}

//������д���ļ�
void CIOCPModel::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	string strFilePath = "C:\\IOServer��־\\";
	//�ж��ļ��Ƿ����
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//Ŀ¼�����ڣ�����Ŀ¼
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			LeaveCriticalSection(&m_csDataFile);
			MessageBox(NULL, L"�޷�������־�ļ�", L"����", MB_OK | MB_ICONERROR);
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	char szTime[20] = { 0 };
	sprintf_s(szTime, "���%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
	strFilePath += szTime;
	//׷��ֻд
	FILE *fe = NULL;
	fopen_s(&fe, strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		m_log.LogMsgToFile(L"���ļ�ʧ�ܣ���������");
		LeaveCriticalSection(&m_csDataFile);
		return;
	}
	//д��ʱ��
	memset(szTime, 0, 20);
	sprintf_s(szTime, "\r\n%02d:%02d:%02d:", st.wHour, st.wMinute, st.wSecond);
	fwrite(szTime, 1, strlen(szTime), fe);
	//ת�����ݸ�ʽ
	string strHexData = ToHexStr(pData, dwLen);
	if (fwrite(strHexData.c_str(), strHexData.size(), 1, fe) < 1)
	{
		m_log.LogMsgToFile(L"д���ļ�ʧ�ܣ���������");
		fclose(fe);
		LeaveCriticalSection(&m_csDataFile);
		return;
	}

	fclose(fe);
	LeaveCriticalSection(&m_csDataFile);
}

//���������߳�
unsigned int CALLBACK CIOCPModel::ThreadAccept(void *lpParam)
{
	CIOCPModel* pTCP = (CIOCPModel*)lpParam;
	//�ͻ����߳̾��
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
			//���ó�ʱʱ��
			int iTimeout = 60000;	//1����
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//�����ͻ����̵߳Ĳ���
				CThreadParameter * paramThread = new CThreadParameter;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//Ϊÿ���ͻ��˴���һ���߳�
				_beginthread(ThreadClient, 0, paramThread);
			}
			else{
				pTCP->m_log.LogMsgToFile(L"setsockopt ʧ��");
				closesocket(ClientSocket);
			}
		}
	}
	return 0;
}

//�ͻ����߳�
void CIOCPModel::ThreadClient(LPVOID lpParam)
{
	//ȡ������
	CThreadParameter *pParam = (CThreadParameter *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	CIOCPModel *pIOCP = pParam->m_model;
	
	//һ������һ���������
	DeviceConnect *pDevConnect = new DeviceConnect;
	//�����߳�ͬ��
	pDevConnect->m_Parent = pIOCP;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	while (iResult > 0)
	{
		//д���ݵ��ļ�
		pIOCP->WriteDataToFile(recvbuf, iResult);
		int nPackageFlag = pDevConnect->CopyDataToBuf(recvbuf, iResult);
		if (nPackageFlag == 1)
		{
			//���յ������İ������Խ���
			pDevConnect->TryAnalysis();
			//�·�ָ��
			DevCommand cmd;
			if (pIOCP->m_pProtocolMgr->m_CmdOrder.GetOrderById(pDevConnect->m_DeviceID, cmd, SendCommand::DEVICE_TYPE::TYPE_BM))
			{
				cout << "������ָ��\n";
				//cmd.GetCommand(vecCommand, DevCommand::CommandType::PARAM);
			}
			else{
				cout << "��������ָ��\n";
				//cmd.SetDeviceID(pDevConnect->m_DeviceID);
				//cmd.GetCommand(vecCommand, DevCommand::CommandType::NO_PARAM);
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//�������İ�����������

		}else if (nPackageFlag == 3){
			//��������ݰ����Ͽ�����
			//cout << "�����֡\n";
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		OutputDebugString(L"�Ͽ�����\n");
	}
	else{
		cout << "recv ʧ�ܣ������룺" << WSAGetLastError() << endl;
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
//	����������
bool CIOCPModel::Start(CProtocolManager* pPM)
{
	m_pProtocolMgr = pPM;
	// ����ϵͳ�˳����¼�֪ͨ
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
		cout << "�޷������������߳�\n";
		return false;
	}
	return true;
}


////////////////////////////////////////////////////////////////////
//	��ʼ����ϵͳ�˳���Ϣ���˳���ɶ˿ں��߳���Դ
void CIOCPModel::Stop()
{
	// ����ر���Ϣ֪ͨ
	SetEvent(m_hShutdownEvent);
	//�ȴ������߳��˳�
	cout << "����ֹͣ���Э��...\n";
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	CloseHandle(m_hAccept);
	m_hAccept = 0;
	
	cout << "���Э����ֹͣ\n";
}

/////////////////////////////////////////////////////////////////////
// �жϿͻ���Socket�Ƿ��Ѿ��Ͽ���������һ����Ч��Socket��Ͷ��WSARecv����������쳣
// ʹ�õķ����ǳ��������socket�������ݣ��ж����socket���õķ���ֵ
// ��Ϊ����ͻ��������쳣�Ͽ�(����ͻ��˱������߰ε����ߵ�)��ʱ�򣬷����������޷��յ��ͻ��˶Ͽ���֪ͨ��

bool CIOCPModel::_IsSocketAlive(SOCKET s)
{
	int nByteSent = send(s, "", 0, 0);
	if (-1 == nByteSent) return false;
	return true;
}

//void CIOCPModel::AddMeterToVec(CGPRSBigMeter meter)
//{
//	EnterCriticalSection(&m_csChannel);
//	//�ж��Ƿ��Ѿ�����
//	vector<CGPRSBigMeter>::iterator iteMeterIsExist = find_if(m_vecMeters.begin(), m_vecMeters.end(), MeterCompare(meter.GetDeviceID()));
//	if (iteMeterIsExist != m_vecMeters.end())
//	{
//		//�Ѿ�����
//		*iteMeterIsExist = meter;
//	}
//	else{
//		//������
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