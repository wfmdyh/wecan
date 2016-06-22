#include "stdafx.h"
#include "ProtocolManager.h"


CProtocolManager::CProtocolManager():
	NAME_BIG("GPRSBigMeter.dll"),
	NAME_CONCENTRATOR("Concentrator")
{
	//m_DataArrive = NULL;
	InitializeCriticalSection(&m_csVecConcentrator);
	InitializeCriticalSection(&m_csVecBigMeters);
	m_hDatabase = NULL;
	m_FlagShutdown = TRUE;
	m_nBigMeterPort = 1733;
	m_nConcentratorPort = 1734;

	m_pTCPModuleBigMeter = new(nothrow) CIOCPModel;
	m_pTCPModuleCon = new(nothrow)TCPModelConcentrator;
}


CProtocolManager::~CProtocolManager()
{
	if (m_pTCPModuleBigMeter != NULL)
	{
		delete m_pTCPModuleBigMeter;
		m_pTCPModuleBigMeter = NULL;
	}
	DeleteCriticalSection(&m_csVecBigMeters);
	DeleteCriticalSection(&m_csVecConcentrator);
}

//���ô������˿�
bool CProtocolManager::SetBigMeterPort(int nPort)
{
	if (nPort < 1)
	{
		cout << "���ô������˿� " << nPort << "ʧ��\n";
		return false;
	}
	m_nBigMeterPort = nPort;
	return true;
}

//��ȡ�������˿�
int CProtocolManager::GetBigMeterPort()
{
	return m_nBigMeterPort;
}

//���ü����������˿�
bool CProtocolManager::SetConcentratorPort(int nPort)
{
	if (nPort < 1)
	{
		cout << "���ü����������˿� " << nPort << "ʧ��\n";
		return false;
	}
	m_nConcentratorPort = nPort;
	return true;
}

//��ȡ�����������˿�
int CProtocolManager::GetConcentratorPort()
{
	return m_nConcentratorPort;
}

//���»��߲���һ���������
void CProtocolManager::UpdateBigMeter(CGPRSBigMeter meter)
{
	bool bExist = false;
	EnterCriticalSection(&m_csVecBigMeters);
	//�ж��豸�Ƿ��Ѿ���ӹ�
	auto ite = m_vecBigMeters.begin();
	for (; ite != m_vecBigMeters.end(); ite++)
	{
		if (ite->GetDeviceID() == meter.GetDeviceID())
		{
			*ite = meter;
			bExist = true;
			break;
		}
	}
	if (!bExist)
	{
		m_vecBigMeters.push_back(meter);
	}
	
	LeaveCriticalSection(&m_csVecBigMeters);
}

//ȡ�ش�������
vector<CGPRSBigMeter> CProtocolManager::GetBigMeters()
{
	EnterCriticalSection(&m_csVecBigMeters);
	vector<CGPRSBigMeter> vecTemp = m_vecBigMeters;
	LeaveCriticalSection(&m_csVecBigMeters);
	return vecTemp;
}

void CProtocolManager::UpdateConcentrator(DevConcentrator con)
{
	EnterCriticalSection(&m_csVecConcentrator);
	bool bExist = false;
	auto ite = m_vecConcentrator.begin();
	for (; ite != m_vecConcentrator.end(); ite++)
	{
		if (ite->m_DeviceID == con.m_DeviceID)
		{
			*ite = con;
			bExist = true;
			break;
		}
	}
	if (!bExist)
	{
		m_vecConcentrator.push_back(con);
	}


	LeaveCriticalSection(&m_csVecConcentrator);
}

vector<DevConcentrator> CProtocolManager::GetConcentrators()
{
	EnterCriticalSection(&m_csVecConcentrator);
	vector<DevConcentrator> vecTemp = m_vecConcentrator;
	LeaveCriticalSection(&m_csVecConcentrator);
	return vecTemp;
}

//���ݿ��߳�
unsigned int CALLBACK ThreadSave(void *lpParam)
{
	CProtocolManager* pProtocolMgr = (CProtocolManager*)lpParam;
	
	while (!pProtocolMgr->m_FlagShutdown)
	{
		pProtocolMgr->m_DBMgr.UpdateMeters(pProtocolMgr);
		Sleep(5000);
	}
	OutputDebugString(L"�洢�߳��˳�\n");
	return 0;
}

void CProtocolManager::Init(fnDataArrive fnDA, vector<CHANNEL_INFO*> *pVecColData)
{
	m_FlagShutdown = FALSE;
	LoadSocketLib();
	//m_DataArrive = fnDA;
	
	//��ȡ��ǰЭ���ͨ��
	if (StartBigMeter(pVecColData))
	{
		cout << "���Э�������ɹ�\n";
	}
	else{
		cout << "���Э������ʧ��\n";
	}
	if (StartConcentrator(pVecColData))
	{
		cout << "������Э�������ɹ�\n";
	}
	else{
		cout << "������Э������ʧ��\n";
	}

	//���ݿ��߳�
	m_hDatabase = (HANDLE)_beginthreadex(NULL, 0, ThreadSave, this, 0, NULL);
}

void CProtocolManager::UnInit()
{
	m_FlagShutdown = TRUE;
	
	cout << "�ȴ����ݿ��߳��˳�\n";
	WaitForSingleObject(m_hDatabase, INFINITE);
	m_DBMgr.Stop();
	//ֹͣ���Э���TCPģ��
	m_pTCPModuleBigMeter->Stop();
	m_pTCPModuleCon->Stop();
	//m_vecTCPModeles.clear();
	//WaitForMultipleObjects(m_vecChannelHandle.size(), &m_vecChannelHandle[0], TRUE, INFINITE);
	//ֹͣ����Ժ����ͷŶ���
	//iteTCPModel = m_vecTCPModeles.begin();
	//for (; iteTCPModel != m_vecTCPModeles.end(); iteTCPModel++)
	//{
		//(*iteTCPModel)->Stop();
	//	delete *iteTCPModel;
//	}
	//m_vecTCPModeles.clear();
	//�ر��߳̾��
	//vector<HANDLE>::iterator iteChannelHandle = m_vecChannelHandle.begin();
	//for (; iteChannelHandle != m_vecChannelHandle.end(); iteChannelHandle++)
	//{
	//	CloseHandle(*iteChannelHandle);
	//}
	//m_vecChannelHandle.clear();
	////�ͷ��ڴ�
	//vector<HANDLE>().swap(m_vecChannelHandle);

	//ж��winsock��
	UnloadSocketLib();
	OutputDebugString(L"�˳����\n");
}

BOOL CProtocolManager::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// ����(һ�㶼�����ܳ���)
	if (NO_ERROR != nResult)
	{
		//m_log.LogMsgToFile(L"��ʼ��WinSock 2.2ʧ�ܣ�");
		return FALSE;
	}

	return TRUE;
}

//����߳�
//unsigned int CALLBACK ThreadBigChannel(void *lpParam)
//{
//	printf("���Э������\n");
//	CProtocolManager* pPM = (CProtocolManager*)lpParam;
//	CIOCPModel* pModule = new CIOCPModel;
//	//pModule->m_DataArrive = m_DataArrive;
//	pModule->SetPort(pPM->GetBigMeterPort());
//	pPM->m_vecTCPModeles.push_back(pModule);
//	//����
//	pModule->Start(pPM);
//
//	return 0;
//}

//�������
bool CProtocolManager::StartBigMeter(vector<CHANNEL_INFO*> *pVecColData)
{
	for (unsigned int i = 0; i<pVecColData->size(); i++)
	{
		string dllFile = pVecColData->at(i)->astrDllFile;
		if (string::npos != dllFile.rfind(NAME_BIG))
		{
			CHANNEL_INFO *pChannel = pVecColData->at(i);
			//�˿ں�
			string strPort = pChannel->mapParam["Port"];
			m_nBigMeterPort = atoi(strPort.c_str());
			//��ǰֻ����һ��ͨ��
			break;
		}
	}

	m_pTCPModuleBigMeter->SetPort(GetBigMeterPort());
	//����
	return m_pTCPModuleBigMeter->Start(this);
}

//����������
bool CProtocolManager::StartConcentrator(vector<CHANNEL_INFO*> *pVecColData)
{
	for (unsigned int i = 0; i<pVecColData->size(); i++)
	{
		string dllFile = pVecColData->at(i)->astrDllFile;
		if (string::npos != dllFile.rfind(NAME_CONCENTRATOR))
		{
			CHANNEL_INFO *pChannel = pVecColData->at(i);
			//�˿ں�
			string strPort = pChannel->mapParam["Port"];
			m_nConcentratorPort = atoi(strPort.c_str());
			
			//��ǰֻ����һ��ͨ��
			break;
		}
	}
	m_pTCPModuleCon->m_nPort = GetConcentratorPort();
	return m_pTCPModuleCon->Start(this);
}