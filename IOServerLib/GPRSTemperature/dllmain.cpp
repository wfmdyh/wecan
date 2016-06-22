// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "GPRSTemperature.h"
#include "IOCPModel.h"
#include "channel.h"
#include "DataDef.h"

//驱动名称
const char* DRIVER_NAME = "GPRSTemperature.dll";
//协议通道数(副本)
vector<CIOCPModel*> g_IOCP;
vector<CHANNEL_INFO*> g_vecChannels;

CRITICAL_SECTION g_csIOCPModel;
//数据采集完毕以后调用的回调函数
fnDataArrive g_DataArrive = NULL;


BOOL APIENTRY DllMain( HMODULE hModule,
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

//通道线程
void ThreadStart(void *lpParam)
{
	CHANNEL_INFO *pChannel = (CHANNEL_INFO*)lpParam;
	//初始值
	vector<DEVICE_INFO*> vecdev = pChannel->vecDevice;
	vector<DEVICE_INFO*>::iterator iteDevice = vecdev.begin();
	for (; iteDevice != vecdev.end(); iteDevice++)
	{
		DEVICE_INFO *pDev = *iteDevice;
		vector<COL_POINT*> vecPoint = pDev->vecPoint;
		vector<COL_POINT*>::iterator itePoint = vecPoint.begin();
		for (; itePoint != vecPoint.end(); itePoint++)
		{
			COL_POINT *pPoint = *itePoint;
			pPoint->varData->vt = VT_R4;
			pPoint->varData->fltVal = -50.0;
		}
	}
	//端口号
	string strPort = pChannel->mapParam["Port"];
	CIOCPModel *pModule = new CIOCPModel;
	//都指向同一个回调函数
	pModule->m_DataArrive = g_DataArrive;
	pModule->SetPort(atoi(strPort.c_str()));
	pModule->LoadSocketLib();


	//启动
	pModule->Start(pChannel);

	EnterCriticalSection(&g_csIOCPModel);
	//添加
	g_IOCP.push_back(pModule);
	LeaveCriticalSection(&g_csIOCPModel);

	_endthread();
}

//从上层传来的数组中，取出自己协议的通道做一个副本，防止多线程时值错乱。
//赋值方式采用回调函数
extern "C" __declspec(dllexport) BOOL IOS_DeviceOpen(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData)
{
	OutputDebugString(L"dllmain进入\n");
	//赋值回调函数
	g_DataArrive = DataArrive;
	//查询属于本协议的通道
	InitializeCriticalSection(&g_csIOCPModel);

	for (unsigned int i=0;i<pVecColData->size();i++)
	{
		string dllFile = pVecColData->at(i)->astrDllFile;
		if(string::npos != dllFile.rfind(DRIVER_NAME))
		{
			CHANNEL_INFO *pChannel = pVecColData->at(i);
			//拷贝一个副本
			CHANNEL_INFO *copyChannel = new CHANNEL_INFO;
			g_vecChannels.push_back(copyChannel);
			*copyChannel = *pChannel;
			//每个通道一个线程
			_beginthread(ThreadStart, 0, copyChannel);
		}
	}
	OutputDebugString(L"dllmain完毕\n");
	return TRUE;
}

extern "C" __declspec(dllexport) void IOS_DeviceClose()
{
	OutputDebugString(L"室温协议正在卸载...\n");
	DeleteCriticalSection(&g_csIOCPModel);
	for (unsigned i = 0; i < g_IOCP.size(); i++)
	{
		CIOCPModel *pModule = g_IOCP.at(i);
		pModule->Stop();
		delete pModule;
	}
	g_IOCP.clear();
	//删除通道副本
	for (unsigned i = 0; i < g_vecChannels.size(); i++)
	{
		delete g_vecChannels.at(i);
	}
	g_vecChannels.clear();
	OutputDebugString(L"室温协议卸载完毕\n");
}

//写数据
BOOL IOS_DeviceWrite(wstring strChannel,wstring strDevice,wstring strPoint,wstring strValue)
{
	return TRUE;
}
