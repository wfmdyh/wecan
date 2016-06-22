// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "channel.h"
#include "ProtocolMgr.h"
#include "DataDef.h"
#include "DataManager.h"

fnShowMessage g_lpfnShowMessage = NULL;


//非托管代码
#pragma unmanaged
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

ProtocolMgr* g_ProMgr = NULL;

extern "C" __declspec(dllexport) BOOL IOS_DeviceOpen(fnDataArrive DataArrive, vector<CHANNEL_INFO*> *pVecColData)
{
	return FALSE;
}

//驱动新版函数，新的工程应该调用该接口
extern "C" __declspec(dllexport) BOOL IOS_DeviceOpenEx(fnShowMessage lpfnShowMessage, DataManager* pMgr)
{
	g_lpfnShowMessage = lpfnShowMessage;
	g_ProMgr = new ProtocolMgr;
	return g_ProMgr->Start(pMgr);
}

extern "C" __declspec(dllexport) void IOS_DeviceClose()
{
	g_ProMgr->Stop();
	delete g_ProMgr;
}

//打印日志
void ShowMessage(const wchar_t* wcsMsg)
{
	if (g_lpfnShowMessage != NULL && wcsMsg != NULL)
	{
		(*g_lpfnShowMessage)(wcsMsg);
	}
}