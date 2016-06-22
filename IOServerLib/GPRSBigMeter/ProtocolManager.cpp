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

//设置大表监听端口
bool CProtocolManager::SetBigMeterPort(int nPort)
{
	if (nPort < 1)
	{
		cout << "设置大表监听端口 " << nPort << "失败\n";
		return false;
	}
	m_nBigMeterPort = nPort;
	return true;
}

//获取大表监听端口
int CProtocolManager::GetBigMeterPort()
{
	return m_nBigMeterPort;
}

//设置集中器监听端口
bool CProtocolManager::SetConcentratorPort(int nPort)
{
	if (nPort < 1)
	{
		cout << "设置集中器监听端口 " << nPort << "失败\n";
		return false;
	}
	m_nConcentratorPort = nPort;
	return true;
}

//获取集中器监听端口
int CProtocolManager::GetConcentratorPort()
{
	return m_nConcentratorPort;
}

//更新或者插入一条大表数据
void CProtocolManager::UpdateBigMeter(CGPRSBigMeter meter)
{
	bool bExist = false;
	EnterCriticalSection(&m_csVecBigMeters);
	//判断设备是否已经添加过
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

//取回大表的数据
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

//数据库线程
unsigned int CALLBACK ThreadSave(void *lpParam)
{
	CProtocolManager* pProtocolMgr = (CProtocolManager*)lpParam;
	
	while (!pProtocolMgr->m_FlagShutdown)
	{
		pProtocolMgr->m_DBMgr.UpdateMeters(pProtocolMgr);
		Sleep(5000);
	}
	OutputDebugString(L"存储线程退出\n");
	return 0;
}

void CProtocolManager::Init(fnDataArrive fnDA, vector<CHANNEL_INFO*> *pVecColData)
{
	m_FlagShutdown = FALSE;
	LoadSocketLib();
	//m_DataArrive = fnDA;
	
	//获取当前协议的通道
	if (StartBigMeter(pVecColData))
	{
		cout << "大表协议启动成功\n";
	}
	else{
		cout << "大表协议启动失败\n";
	}
	if (StartConcentrator(pVecColData))
	{
		cout << "集中器协议启动成功\n";
	}
	else{
		cout << "集中器协议启动失败\n";
	}

	//数据库线程
	m_hDatabase = (HANDLE)_beginthreadex(NULL, 0, ThreadSave, this, 0, NULL);
}

void CProtocolManager::UnInit()
{
	m_FlagShutdown = TRUE;
	
	cout << "等待数据库线程退出\n";
	WaitForSingleObject(m_hDatabase, INFINITE);
	m_DBMgr.Stop();
	//停止大表协议的TCP模型
	m_pTCPModuleBigMeter->Stop();
	m_pTCPModuleCon->Stop();
	//m_vecTCPModeles.clear();
	//WaitForMultipleObjects(m_vecChannelHandle.size(), &m_vecChannelHandle[0], TRUE, INFINITE);
	//停止完毕以后，在释放对象
	//iteTCPModel = m_vecTCPModeles.begin();
	//for (; iteTCPModel != m_vecTCPModeles.end(); iteTCPModel++)
	//{
		//(*iteTCPModel)->Stop();
	//	delete *iteTCPModel;
//	}
	//m_vecTCPModeles.clear();
	//关闭线程句柄
	//vector<HANDLE>::iterator iteChannelHandle = m_vecChannelHandle.begin();
	//for (; iteChannelHandle != m_vecChannelHandle.end(); iteChannelHandle++)
	//{
	//	CloseHandle(*iteChannelHandle);
	//}
	//m_vecChannelHandle.clear();
	////释放内存
	//vector<HANDLE>().swap(m_vecChannelHandle);

	//卸载winsock库
	UnloadSocketLib();
	OutputDebugString(L"退出完毕\n");
}

BOOL CProtocolManager::LoadSocketLib()
{
	WSADATA wsaData;
	int nResult;
	nResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	// 错误(一般都不可能出现)
	if (NO_ERROR != nResult)
	{
		//m_log.LogMsgToFile(L"初始化WinSock 2.2失败！");
		return FALSE;
	}

	return TRUE;
}

//大表线程
//unsigned int CALLBACK ThreadBigChannel(void *lpParam)
//{
//	printf("大表协议启动\n");
//	CProtocolManager* pPM = (CProtocolManager*)lpParam;
//	CIOCPModel* pModule = new CIOCPModel;
//	//pModule->m_DataArrive = m_DataArrive;
//	pModule->SetPort(pPM->GetBigMeterPort());
//	pPM->m_vecTCPModeles.push_back(pModule);
//	//启动
//	pModule->Start(pPM);
//
//	return 0;
//}

//启动大表
bool CProtocolManager::StartBigMeter(vector<CHANNEL_INFO*> *pVecColData)
{
	for (unsigned int i = 0; i<pVecColData->size(); i++)
	{
		string dllFile = pVecColData->at(i)->astrDllFile;
		if (string::npos != dllFile.rfind(NAME_BIG))
		{
			CHANNEL_INFO *pChannel = pVecColData->at(i);
			//端口号
			string strPort = pChannel->mapParam["Port"];
			m_nBigMeterPort = atoi(strPort.c_str());
			//当前只允许一个通道
			break;
		}
	}

	m_pTCPModuleBigMeter->SetPort(GetBigMeterPort());
	//启动
	return m_pTCPModuleBigMeter->Start(this);
}

//启动集中器
bool CProtocolManager::StartConcentrator(vector<CHANNEL_INFO*> *pVecColData)
{
	for (unsigned int i = 0; i<pVecColData->size(); i++)
	{
		string dllFile = pVecColData->at(i)->astrDllFile;
		if (string::npos != dllFile.rfind(NAME_CONCENTRATOR))
		{
			CHANNEL_INFO *pChannel = pVecColData->at(i);
			//端口号
			string strPort = pChannel->mapParam["Port"];
			m_nConcentratorPort = atoi(strPort.c_str());
			
			//当前只允许一个通道
			break;
		}
	}
	m_pTCPModuleCon->m_nPort = GetConcentratorPort();
	return m_pTCPModuleCon->Start(this);
}