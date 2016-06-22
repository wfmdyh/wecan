#include "stdafx.h"
#include "GPRSTemperature.h"
#include "IOCPModel.h"
#include "DeviceConnect.h"

//recv�Ĵ�С
#define DEFAULT_BUFLEN 1024

//�����С
#define DATA_BUF_SIZE					(1024*1024*10)

// �ͷž����
#define RELEASE_HANDLE(x)               {if(x != NULL && x!=INVALID_HANDLE_VALUE){ CloseHandle(x);x = NULL;}}

//�̲߳���
class CThreadParameter
{
public:
	CIOCPModel *m_model;
	SOCKET m_sockClient;
};

//�ص���������������һ���豸�Ժ������̻߳��Զ�����
void CIOCPModel::GetOneDevice(CGPRSTemperature *pDevice)
{
	EnterCriticalSection(&m_csChannel);
	OutputDebugString(L"GetOneDevice�ص�\n");
	//���յ����������ݣ����µ�ͨ����
	for (unsigned i = 0; i < m_pChannel->vecDevice.size(); i++)
	{
		//�ҵ���Ӧ���豸
		DEVICE_INFO *pDev = m_pChannel->vecDevice.at(i);
		if (pDev->astrDeviceName.compare(pDevice->GetDeviceID()) == 0)
		{
			//ֻȡ��һ��
			COL_POINT *pPoint = pDev->vecPoint.at(0);
			VariantCopy(pPoint->varData, &(pDevice->m_values[RI_Temperature]));
			//WF 3-10
			pPoint->m_IsChange = true;
			break;
		}
	}
	LeaveCriticalSection(&m_csChannel);
	//����IOServer�Ļص�����
	(*m_DataArrive)(m_pChannel);
	
	
}

CIOCPModel::CIOCPModel(void)
{
	m_hThrdAccept = NULL;
	m_ListenSocket = INVALID_SOCKET;
	
	m_DataArrive = NULL;
	//m_vecDevice = NULL;
	m_log.SetFileName(L"����");
}


CIOCPModel::~CIOCPModel(void)
{
	// ȷ����Դ�����ͷ�
	//this->Stop();
}

//������д���ļ�
void CIOCPModel::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	string strFilePath = "C://IOServer��־//";
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
	sprintf_s(szTime, "����%d_%02d_%02d.data", st.wYear, st.wMonth, st.wDay);
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
	if (fwrite(pData, dwLen, 1, fe) < 1)
	{
		m_log.LogMsgToFile(L"д���ļ�ʧ�ܣ���������");
		fclose(fe);
		LeaveCriticalSection(&m_csDataFile);
		return;
	}

	fclose(fe);
	LeaveCriticalSection(&m_csDataFile);
}

//====================================================================================
//
//				    ϵͳ��ʼ������ֹ
//
//====================================================================================




////////////////////////////////////////////////////////////////////
// ��ʼ��WinSock 2.2
bool CIOCPModel::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
		m_log.LogMsgToFile(L"��ʼ��WinSock 2.2ʧ�ܣ�");
		return false;
	}

	return true;
}

//ɨ�������߳�
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

//�ͻ����߳�
void CIOCPModel::ThreadClient(LPVOID lpParam)
{
	OutputDebugString(L"����ͻ����߳�\n");
	//ȡ������
	CThreadParameter *pParam = (CThreadParameter *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	CIOCPModel *pIOCP = pParam->m_model;

	//һ������һ���������
	DeviceConnect *pDevConnect = new DeviceConnect;
	//�����߳�ͬ��
	pDevConnect->m_Parent = pIOCP;
	//���ݻص�����
	//pDevConnect->m_GetOneDevice = GetOneDevice;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int recvbuflen = DEFAULT_BUFLEN;

	int iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	while (iResult > 0)
	{
		//д���ݵ��ļ�
		pIOCP->WriteDataToFile(recvbuf, iResult);
		pDevConnect->CopyDataToBuf(recvbuf, iResult);
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		OutputDebugString(L"�Ͽ�����\n");
	}
	else{
		//printf("recv failed with error: %d\n", WSAGetLastError());
		OutputDebugString(L"recv ʧ��\n");
	}
	
	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	// cleanup
	closesocket(ClientSocket);
	delete pDevConnect;
	pDevConnect = NULL;
	_endthread();
	OutputDebugString(L"�뿪�ͻ����߳�\n");
}

//accept�߳�
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
			//���ó�ʱʱ��
			int iTimeout = 5000;	//3s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				/*THREADPARAMS_CLIENT* tc = new THREADPARAMS_CLIENT;
				tc->pIOCP = pIOCP;
				tc->sockClient = ClientSocket;*/
				//�����ͻ����̵߳Ĳ���
				CThreadParameter * paramThread = new CThreadParameter;
				paramThread->m_model = pIOCP;
				paramThread->m_sockClient = ClientSocket;
				//Ϊÿ���ͻ��˴���һ���߳�
				_beginthread(ThreadClient, 0, paramThread);
			}
			else{
				pIOCP->m_log.LogMsgToFile(L"setsockopt ʧ��");
				closesocket(ClientSocket);
			}
		}
	}
	WSACleanup();
	_endthread();
}

//////////////////////////////////////////////////////////////////
//	����������
BOOL CIOCPModel::Start(CHANNEL_INFO* pChannel)
{
	m_pChannel = pChannel;
	// ��ʼ���̻߳�����
	InitializeCriticalSection(&m_csDataFile);
	InitializeCriticalSection(&m_csChannel);

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

	//����accept�߳�
	m_hThrdAccept = (HANDLE)_beginthread(ThreadAccept, 0, this);
	if (m_hThrdAccept == NULL)
	{
		closesocket(m_ListenSocket);
		WSACleanup();
		return FALSE;
	}

	OutputDebugString(L"ϵͳ׼���������Ⱥ�����....\n");

	return TRUE;
}


////////////////////////////////////////////////////////////////////
//	��ʼ����ϵͳ�˳���Ϣ���˳���ɶ˿ں��߳���Դ
void CIOCPModel::Stop()
{
		// ����ر���Ϣ֪ͨ
		SetEvent(m_hShutdownEvent);

		//// �ȴ����еĿͻ�����Դ�˳�
		//WaitForMultipleObjects(m_nThreads, m_phWorkerThreads, TRUE, INFINITE);
		closesocket(m_ListenSocket);
		WSACleanup();
		TerminateThread(m_hThrdAccept, 0);
	

		// ɾ���ͻ����б�Ļ�����
		DeleteCriticalSection(&m_csDataFile);
		DeleteCriticalSection(&m_csChannel);

		// �ر�ϵͳ�˳��¼����
		RELEASE_HANDLE(m_hShutdownEvent);

		//this->_ShowMessage("ֹͣ����\n");
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

//�ɼ�ʱ������趨ʱ�䣬�͸���ʼֵ
void CIOCPModel::Init50(time_t NowTime)
{
	//map<string, CGPRSTemperature*>::iterator it;
	//CGPRSTemperature *p_temp;

	//for (it = (*m_vecDevice).begin(); it != (*m_vecDevice).end() && m_uiDeviceCount > 0; ++it)
	//{
	//	p_temp = it->second;
	//	if (NULL == p_temp)
	//		continue;

	//	if (difftime(NowTime, p_temp->LastTime) > 7200)// 7200�룬��������Сʱ����Ϊ����
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