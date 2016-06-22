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
	m_log.SetFileName(L"ˮ������");
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

	m_hAccept = (HANDLE)_beginthreadex(NULL, 0, ThreadAccept2, this, 0, NULL);
	if (m_hAccept == 0)
	{
		cout << "�޷������������߳�\n";
		return false;
	}
	return true;
}

void TCPModelConcentrator::Stop()
{
	// ����ر���Ϣ֪ͨ
	SetEvent(m_hShutdownEvent);
	//�ȴ������߳��˳�
	cout << "����ֹͣ������Э��...\n";
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	if (m_hAccept != 0)
	{
		CloseHandle(m_hAccept);
	}
	m_hAccept = 0;

	cout << "������Э����ֹͣ\n";
}


//���������߳�
unsigned int CALLBACK TCPModelConcentrator::ThreadAccept2(void *lpParam)
{
	TCPModelConcentrator* pTCP = (TCPModelConcentrator*)lpParam;
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
			int iTimeout = 60000;	//60s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//�����ͻ����̵߳Ĳ���
				CThreadParameter2 * paramThread = new CThreadParameter2;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//Ϊÿ���ͻ��˴���һ���߳�
				_beginthread(ThreadClient2, 0, paramThread);
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
void TCPModelConcentrator::ThreadClient2(LPVOID lpParam)
{
	//ȡ������
	CThreadParameter2 *pParam = (CThreadParameter2 *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	TCPModelConcentrator *pIOCP = pParam->m_model;

	//һ������һ���������
	DeviceConnectCon *pDevConnect = new(nothrow) DeviceConnectCon;
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
			//���������е�֡�����Խ���
			pDevConnect->TryAnalysis();
			//�·�ָ��
			DevCommand cmd;
			if (pIOCP->m_CmdOrder.GetOrderById(pDevConnect->m_DeviceID, cmd, SendCommand::DEVICE_TYPE::TYPE_CON))
			{
				cout << "��������������ָ��\n";
			}
			else{
				cout << "����������������ָ��\n";
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//�������İ�����������
			cout << "�������İ����ȴ���������" << endl;
		}
		else if (nPackageFlag == 3){
			//��������ݰ����Ͽ�����
			cout << "�����֡\n";
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		cout << "�Ͽ�����\n";
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

//������д���ļ�
void TCPModelConcentrator::WriteDataToFile(char *pData, DWORD dwLen)
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
	string strFileName;
	stringstream ss;
	ss << "������_" << st.wYear << "_" << st.wMonth << "_" << st.wDay << ".data";
	strFileName = ss.str();
	strFilePath += strFileName;
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
	string strTime;
	ss.clear();
	ss.str("");
	ss << "\r\n" << st.wHour << ":" << st.wMinute << ":" << st.wSecond << ":";
	strTime = ss.str();
	fwrite(strTime.c_str(), 1, strTime.size(), fe);
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