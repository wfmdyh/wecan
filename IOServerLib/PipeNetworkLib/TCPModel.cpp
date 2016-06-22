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
	/*m_log.SetFileName(L"ˮ������");*/
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
		ShowMessage(L"��ʼ��winsock��ʧ�ܣ��������ϵͳ�Ƿ�������ȷ");
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
		ShowMessage(L"�޷������������߳�");
		return FALSE;
	}
	return TRUE;
}

void TCPModel::Stop()
{
	
	//�ȴ������߳��˳�
	ShowMessage(L"����ֹͣЭ��...");
	shutdown(m_ListenSocket, SD_BOTH);
	closesocket(m_ListenSocket);
	WaitForSingleObject(m_hAccept, INFINITE);
	if (m_hAccept != 0)
	{
		CloseHandle(m_hAccept);
	}
	m_hAccept = 0;

	ShowMessage(L"Э����ֹͣ");
}


//���������߳�
unsigned int CALLBACK TCPModel::ThreadAccept(void *lpParam)
{
	wstringstream wss;
	wstring strMsg;
	TCPModel* pTCP = (TCPModel*)lpParam;
	//�ͻ����߳̾��
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
			//���ó�ʱʱ��
			int iTimeout = 5000;	//5s
			int ret = setsockopt(ClientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			ret = setsockopt(ClientSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&iTimeout, sizeof(iTimeout));
			if (ret == 0)
			{
				//�����ͻ����̵߳Ĳ���
				THREAD_PARAM * paramThread = new THREAD_PARAM;
				paramThread->m_model = pTCP;
				paramThread->m_sockClient = ClientSocket;
				//Ϊÿ���ͻ��˴���һ���߳�
				_beginthread(ThreadClient, 0, paramThread);
			}
			else{
				ShowMessage(L"setsockopt ʧ��");
				closesocket(ClientSocket);
			}
		}
	}
	return 0;
}

//�ͻ����߳�
void TCPModel::ThreadClient(LPVOID lpParam)
{
	wstringstream wss;
	wstring strMsg;

	////ȡ������
	THREAD_PARAM *pParam = (THREAD_PARAM *)lpParam;
	SOCKET ClientSocket = pParam->m_sockClient;
	TCPModel *pIOCP = pParam->m_model;

	//һ������һ���������
	DeviceConnect *pDevConnect = new(nothrow)DeviceConnect;
	//�����߳�ͬ��
	//pDevConnect->m_Parent = pIOCP;
	const int recvbuflen = 1024;
	char recvbuf[recvbuflen] = { 0 };
	

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
			//���и�ֵ
			pIOCP->m_pProtocolMgr->UpdateDevice(pDevConnect->m_Device);
			//�·�ָ��
			DevCommand cmd;
			pIOCP->m_pCmdMgr->GetCommandByID(pDevConnect->m_DeviceID, cmd);
			if (cmd.GetCmdType() == DevCommand::PARAM)
			{
				ShowMessage(L"�������������ָ��");
			}
			else{
				ShowMessage(L"���������������ָ��");
			}
			vector<BYTE> vecCommand = cmd.GetHexCommand();
			char *bySend = (char*)&vecCommand[0];
			int nSendLen = vecCommand.size();
			send(ClientSocket, bySend, nSendLen, 0);
		}
		else if (nPackageFlag == 2){
			//�������İ�����������
			ShowMessage(L"�������İ����ȴ���������");
		}
		else if (nPackageFlag == 3){
			//��������ݰ����Ͽ�����
			ShowMessage(L"�����֡");
			iResult = -1;
			break;
		}
		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
	}
	if (iResult == 0){
		ShowMessage(L"�Ͽ�����");
	}
	else{
		wss.clear();
		wss << L"recv ʧ�ܣ������룺" << WSAGetLastError();
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

//������д���ļ�
void TCPModel::WriteDataToFile(char *pData, DWORD dwLen)
{
	EnterCriticalSection(&m_csDataFile);
	wstringstream wss;
	wstring strMsg;

	string strFilePath = "C:\\IOServer��־\\";
	//�ж��ļ��Ƿ����
	if (_access(strFilePath.c_str(), 0) == -1)
	{
		//Ŀ¼�����ڣ�����Ŀ¼
		if (_mkdir(strFilePath.c_str()) == -1)
		{
			LeaveCriticalSection(&m_csDataFile);
			ShowMessage(L"�޷�������־�ļ�");
			return;
		}
	}

	SYSTEMTIME st;
	GetLocalTime(&st);
	string strFileName;
	stringstream ss;
	
	ss << "�����_" << st.wYear << "_";
	ss << setw(2) << setfill('0') << st.wMonth;
	ss << "_";
	ss << setw(2) << setfill('0') << st.wDay;
	ss << ".data";
	strFileName = ss.str();
	strFilePath += strFileName;
	//׷��ֻд
	FILE *fe = NULL;
	fopen_s(&fe, strFilePath.c_str(), "ab");
	if (fe == NULL)
	{
		ShowMessage(L"���ļ�ʧ�ܣ������");
		LeaveCriticalSection(&m_csDataFile);
		return;
	}
	//д��ʱ��
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
	//ת�����ݸ�ʽ
	string strHexData = m_tools.ToHexStr(pData, dwLen);
	if (fwrite(strHexData.c_str(), strHexData.size(), 1, fe) < 1)
	{
		ShowMessage(L"д���ļ�ʧ�ܣ������");
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