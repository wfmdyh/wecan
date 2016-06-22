// dllmain.cpp : ���� DLL Ӧ�ó������ڵ㡣
#include "stdafx.h"
#include "GPRSBigMeter.h"
#include "IOCPModel.h"
#include "channel.h"
#include "DataDef.h"
#include "DataBaseManager.h"
#include "ProtocolManager.h"

CProtocolManager *g_pProtocolMgr = new CProtocolManager;

//��������
//const char* DRIVER_NAME = "GPRSBigMeter.dll";
//Э��ͨ����(����)
//vector<CIOCPModel*> g_IOCP;
//vector<CHANNEL_INFO*> g_vecChannels;
//���ݿ������
//CDataBaseManager *g_dbManager = NULL;

//CRITICAL_SECTION g_csIOCPModel;
//���ݲɼ�����Ժ���õĻص�����
//fnDataArrive g_DataArrive = NULL;


BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
	)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

//ͨ���߳�
//void ThreadStart(void *lpParam)
//{
//	CHANNEL_INFO *pChannel = (CHANNEL_INFO*)lpParam;
//	//�˿ں�
//	string strPort = pChannel->mapParam["Port"];
//	CIOCPModel *pModule = new CIOCPModel;
//	//��ָ��ͬһ���ص�����
//	pModule->m_DataArrive = g_DataArrive;
//	pModule->SetPort(atoi(strPort.c_str()));
//	pModule->LoadSocketLib();
//
//	//����
//	pModule->Start(pChannel);
//
//	EnterCriticalSection(&g_csIOCPModel);
//	//���
//	g_IOCP.push_back(pModule);
//	LeaveCriticalSection(&g_csIOCPModel);
//
//	_endthread();
//}

//���ϲ㴫���������У�ȡ���Լ�Э���ͨ����һ����������ֹ���߳�ʱֵ���ҡ�
//��ֵ��ʽ���ûص�����
extern "C" __declspec(dllexport) BOOL IOS_DeviceOpen(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData)
{
	
	g_pProtocolMgr->Init(DataArrive, pVecColData);
	//��ֵ�ص�����
	//g_DataArrive = DataArrive;
	//��ѯ���ڱ�Э���ͨ��
	//InitializeCriticalSection(&g_csIOCPModel);

	//for (unsigned int i = 0; i<pVecColData->size(); i++)
	//{
	//	string dllFile = pVecColData->at(i)->astrDllFile;
	//	if (string::npos != dllFile.rfind(DRIVER_NAME))
	//	{
	//		CHANNEL_INFO *pChannel = pVecColData->at(i);
	//		//����һ������
	//		CHANNEL_INFO *copyChannel = new CHANNEL_INFO;
	//		g_vecChannels.push_back(copyChannel);
	//		*copyChannel = *pChannel;
	//		//ÿ��ͨ��һ���߳�
	//		_beginthread(ThreadStart, 0, copyChannel);
	//	}
	//}
	//���ݿ�
	//g_dbManager = new CDataBaseManager;
	//g_dbManager->Start(g_vecChannels);

	return TRUE;
}

extern "C" __declspec(dllexport) void IOS_DeviceClose()
{
	g_pProtocolMgr->UnInit();
	delete g_pProtocolMgr;
	g_pProtocolMgr = NULL;
	//���ݿ�
	/*g_dbManager->Stop();
	if (g_dbManager != NULL)
	{
		delete g_dbManager;
		g_dbManager = NULL;
	}*/

	/*for (unsigned i = 0; i < g_IOCP.size(); i++)
	{
		CIOCPModel *pModule = g_IOCP.at(i);
		pModule->Stop();
		delete pModule;
	}
	g_IOCP.clear();*/
	//ɾ��ͨ������
	/*for (unsigned i = 0; i < g_vecChannels.size(); i++)
	{
		delete g_vecChannels.at(i);
	}
	g_vecChannels.clear();*/

	//DeleteCriticalSection(&g_csIOCPModel);
}

//д����
BOOL IOS_DeviceWrite(wstring strChannel, wstring strDevice, wstring strPoint, wstring strValue)
{
	return TRUE;
}
